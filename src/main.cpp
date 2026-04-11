/*
 * VISLA Modbus Display — ESP32-S3-Touch-LCD-4.3B
 * Viessmann Energycal Slim W — Controllo con display touch
 *
 * Controllo via:
 *   1. Display touch LVGL 800x480
 *   2. API REST via WiFi (http://<ip>/api/...)
 *   3. Web pages: /test (bottoni test) e /sniffer (ascolto passivo)
 *   4. Seriale USB (comandi testuali)
 *
 * MODALITÀ SNIFFER:
 *   Decommentare #define SNIFFER_MODE per attivare modalità ascolto passivo
 *   Non invierà comandi, solo ascolterà il master originale
 */

// #define SNIFFER_MODE  // Decommentare per attivare modalità sniffer passivo

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include <Wire.h>

using namespace esp_panel::drivers;
using namespace esp_panel::board;

// === WiFi Config ===
const char* WIFI_SSID = "Molinella";
const char* WIFI_PASS = "Fastweb10";

// === RS485 Pin Definitions (Waveshare ESP32-S3-Touch-LCD-4.3B) ===
#define RS485_TX_PIN  44
#define RS485_RX_PIN  43
#define RS485          Serial1
#define BAUD_RATE         9600
#define SEND_INTERVAL     68000  // 68 sec (come master originale)

// === MODBUS RETRY CONFIGURATION ===
#define MODBUS_RETRIES    10     // Numero di ripetizioni per registro
#define MODBUS_RETRY_DELAY 50    // Delay tra ripetizioni (ms)

// === Stato ventilconvettore ===
uint16_t regConfig = 0x4003;  // FREDDO MAX (come master originale) - bit14=FREDDO, bit1-0=FAN MAX
uint16_t regTemp   = 0x00CD;  // 20.5C (x10)
uint16_t regMode   = 0xaf;    // modo stagionale (identico al master originale)
bool     powerOn   = true;    // acceso al boot
bool     heating   = false;   // modalità FREDDO (non CALDO)

// === Temperature Debounce (aspetta 1 secondo prima di inviare) ===
uint32_t tempChangeTime = 0;
#define TEMP_DEBOUNCE_MS 1000  // Aspetta 1 secondo
float pendingTemp = 0;
bool tempPending = false;

// === Web Server ===
WebServer webServer(80);

// === Sniffer Mode - Frame Capture Buffer ===
#define SNIFFER_BUFFER_SIZE 100
struct SniffedFrame {
  uint32_t timestamp;
  uint8_t addr;
  uint8_t func;
  uint16_t reg;
  uint16_t val;
  uint8_t lrc;
  bool lrc_ok;
};
SniffedFrame sniffBuffer[SNIFFER_BUFFER_SIZE];
int sniffIndex = 0;

// === Command Log (Registri inviati) ===
#define CMD_LOG_SIZE 50
struct CommandLog {
  uint32_t timestamp;
  uint16_t reg;
  uint16_t val;
};
CommandLog cmdLog[CMD_LOG_SIZE];
int cmdLogIndex = 0;

// === Sniffer Annotations (saved server-side) ===
#include <map>
std::map<String, String> sniffAnnotations;

// === LVGL UI elements ===
static lv_obj_t *label_temp;
static lv_obj_t *btn_on, *btn_off;
static lv_obj_t *btn_heat, *btn_cool;
static lv_obj_t *btn_fan[4];
static lv_obj_t *label_status;
static lv_obj_t *label_wifi;

// === Colori LVGL ===
#define COLOR_BG       lv_color_hex(0x1a1a2e)
#define COLOR_CARD     lv_color_hex(0x16213e)
#define COLOR_ACCENT   lv_color_hex(0xe94560)
#define COLOR_COOL     lv_color_hex(0x0f3460)
#define COLOR_INACTIVE lv_color_hex(0x444444)
#define COLOR_TEXT     lv_color_hex(0xeeeeee)
#define COLOR_DIM      lv_color_hex(0xaaaaaa)

// ============================================================
//  Modbus ASCII
// ============================================================

uint8_t calculateLRC(uint8_t *data, int len) {
  uint8_t lrc = 0;
  for (int i = 0; i < len; i++) lrc += data[i];
  return (uint8_t)(-(int8_t)lrc);
}

bool parseModbusFrame(const char* hexStr, SniffedFrame* frame) {
  // Parsing frame ASCII: AAFFRRRRRRRRLLCC (6 bytes payload + lrc = 7 bytes total)
  int len = strlen(hexStr);
  if (len < 14) return false;

  uint8_t data[7];
  for (int i = 0; i < 7 && i*2 < len; i++) {
    char hex[3] = {hexStr[i*2], hexStr[i*2+1], 0};
    data[i] = (uint8_t)strtol(hex, NULL, 16);
  }

  frame->timestamp = millis();
  frame->addr = data[0];
  frame->func = data[1];
  frame->reg = (data[2] << 8) | data[3];
  frame->val = (data[4] << 8) | data[5];
  frame->lrc = data[6];

  // Verify LRC
  uint8_t lrc_calc = calculateLRC(data, 6);
  frame->lrc_ok = (frame->lrc == lrc_calc);

  return true;
}

void modbusWriteRegister(uint8_t addr, uint16_t reg, uint16_t value) {
  uint8_t payload[6];
  payload[0] = addr;
  payload[1] = 0x06;
  payload[2] = (reg >> 8) & 0xFF;
  payload[3] = reg & 0xFF;
  payload[4] = (value >> 8) & 0xFF;
  payload[5] = value & 0xFF;

  uint8_t lrc = calculateLRC(payload, 6);

  char txBuf[32];
  int pos = 0;
  txBuf[pos++] = ':';
  for (int i = 0; i < 6; i++) pos += sprintf(&txBuf[pos], "%02X", payload[i]);
  pos += sprintf(&txBuf[pos], "%02X", lrc);
  txBuf[pos++] = '\r';
  txBuf[pos++] = '\n';

  // Log comando inviato
  cmdLog[cmdLogIndex % CMD_LOG_SIZE].timestamp = millis();
  cmdLog[cmdLogIndex % CMD_LOG_SIZE].reg = reg;
  cmdLog[cmdLogIndex % CMD_LOG_SIZE].val = value;
  cmdLogIndex++;

  while (RS485.available()) RS485.read();
  RS485.write((uint8_t*)txBuf, pos);
  RS485.flush();
}

void sendAllRegisters() {
#ifdef SNIFFER_MODE
  Serial.println(">>> [SNIFFER MODE] Invio disabilitato, ascolto passivo...");
  return;
#endif
  Serial.printf(">>> Invio registri (x%d con delay %dms)...\n", MODBUS_RETRIES, MODBUS_RETRY_DELAY);

  // REG 101 - invia MODBUS_RETRIES volte
  for (int i = 0; i < MODBUS_RETRIES; i++) {
    modbusWriteRegister(0, 101, regConfig);
    delay(MODBUS_RETRY_DELAY);
  }
  delay(1160);

  // REG 102 - invia MODBUS_RETRIES volte
  for (int i = 0; i < MODBUS_RETRIES; i++) {
    modbusWriteRegister(0, 102, regTemp);
    delay(MODBUS_RETRY_DELAY);
  }
  delay(1160);

  // REG 103 - invia MODBUS_RETRIES volte
  for (int i = 0; i < MODBUS_RETRIES; i++) {
    modbusWriteRegister(0, 103, regMode);
    delay(MODBUS_RETRY_DELAY);
  }

  Serial.printf("    101=0x%04X 102=0x%04X(%.1fC) 103=0x%04X OK (sent x%d)\n",
                regConfig, regTemp, regTemp / 10.0, regMode, MODBUS_RETRIES);
}

// === Helper ===

bool isOn()      { return powerOn; }
int  fanSpeed()  { return regConfig & 0x03; }
bool isHeating() { return heating; }
float getTemp()  { return regTemp / 10.0; }

const char* fanName() {
  const char* names[] = {"auto", "min", "night", "max"};
  return names[fanSpeed()];
}

String statusJSON() {
  String json = "{";
  json += "\"power\":\"" + String(isOn() ? "on" : "off") + "\",";
  json += "\"temperature\":" + String(getTemp(), 1) + ",";
  json += "\"fan\":\"" + String(fanName()) + "\",";
  json += "\"fan_speed\":" + String(fanSpeed()) + ",";
  json += "\"mode\":\"" + String(isHeating() ? "heat" : "cool") + "\",";
  json += "\"reg101\":\"0x" + String(regConfig, HEX) + "\",";
  json += "\"reg102\":\"0x" + String(regTemp, HEX) + "\",";
  json += "\"reg103\":\"0x" + String(regMode, HEX) + "\"";
  json += "}";
  return json;
}

// ============================================================
//  Azioni di controllo
// ============================================================

void setPower(bool on) {
  // === POWER CONTROL - Cambia solo BIT 7 ===
  // BIT 7 (0x0080): Standby flag
  //   0 = Acceso (ON)
  //   1 = Spento (OFF/Standby)
  //
  // Gli altri bit rimangono INVARIATI:
  //   BIT 0-1: Fan speed (max, medio, basso, off)
  //   BIT 13-14: Modalità (caldo/freddo)
  //   REG 102: Temperatura (NON tocca)

  if (on) {
    regConfig &= ~0x0080;  // Cancella bit 7 → accendi
  } else {
    regConfig |= 0x0080;   // Setta bit 7 → spegni
  }
  powerOn = on;
  tempPending = false;
  Serial.printf("[POWER] %s (regConfig=0x%04X)\n", on ? "ON" : "OFF", regConfig);
  sendAllRegisters();
}

void setFanSpeed(int speed) {
  // === FAN SPEED CONTROL - Cambia solo BIT 0-1 ===
  // BIT 0-1 (0x0003): Velocità ventola
  //   00 = Off
  //   01 = Basso
  //   10 = Medio
  //   11 = Max
  //
  // Gli altri bit rimangono INVARIATI:
  //   BIT 7: Power (acceso/spento)
  //   BIT 13-14: Modalità (caldo/freddo)
  //   REG 102: Temperatura (NON tocca)

  if (speed < 0 || speed > 3) return;
  regConfig = (regConfig & ~0x03) | (speed & 0x03);  // Cancella bit 0-1, poi setta il nuovo speed
  sendAllRegisters();
}

void setMode(bool heat) {
  // === MODE CONTROL (Caldo/Freddo) - Cambia solo BIT 13-14 ===
  // BIT 13-14 in regConfig: Modalità
  //   BIT 14=1, BIT 13=0: FREDDO (cooling) - valore 0x4000
  //   BIT 14=0, BIT 13=1: CALDO (heating) - valore 0x2000
  //
  // BIT 1 in regMode: Indicatore modalità interna
  //
  // Gli altri bit rimangono INVARIATI:
  //   BIT 0-1: Fan speed
  //   BIT 7: Power (acceso/spento)
  //   REG 102: Temperatura (NON tocca)

  heating = heat;
  if (heat) {
    regMode |= 0x02;  // Setta bit 1 in regMode (modalità caldo)
    if (powerOn) {
      regConfig &= ~(1 << 14);  // Cancella bit 14
      regConfig |= (1 << 13);   // Setta bit 13 → CALDO
    }
  } else {
    regMode &= ~0x02;  // Cancella bit 1 in regMode (modalità freddo)
    if (powerOn) {
      regConfig &= ~(1 << 13);  // Cancella bit 13
      regConfig |= (1 << 14);   // Setta bit 14 → FREDDO
    }
  }
  sendAllRegisters();
}

void setTemperature(float temp) {
  // === TEMPERATURE CONTROL - Modifica solo REG 102 ===
  // REG 102: Temperatura (0x00CD = 20.5°C, 0x0032 = 5.0°C, etc)
  // Formato: temperatura * 10 (es: 20.5°C → 0x00CD = 205)
  //
  // Gli altri registri rimangono INVARIATI:
  //   REG 101 (regConfig): Power, fan speed, modalità (NON tocca)
  //   REG 103 (regMode): Modo stagionale (NON tocca)
  //
  // DEBOUNCE: L'utente può cambiare temperatura rapidamente
  //   - Display aggiorna SUBITO (esperienza reattiva)
  //   - Ma Modbus invia solo dopo 1 sec di inattività
  //   (evita di inviare tanti comandi quando l'utente scorre)

  if (temp < 16.0) temp = 16.0;
  if (temp > 28.0) temp = 28.0;  // Limiti: 16-28°C

  regTemp = (uint16_t)(temp * 10);  // Aggiorna subito per il display
  pendingTemp = temp;
  tempPending = true;
  tempChangeTime = millis();

  Serial.printf("[TEMP] Display: %.1f°C, Modbus invio in 1 sec se non cambi\n", temp);
}

// Forward declaration
void updateUI();

// ============================================================
//  LVGL UI — Touch interface
// ============================================================

static lv_style_t style_card;
static lv_style_t style_btn;
static lv_style_t style_btn_active;
static lv_style_t style_btn_cool;

static void init_styles() {
  lv_style_init(&style_card);
  lv_style_set_bg_color(&style_card, COLOR_CARD);
  lv_style_set_radius(&style_card, 12);
  lv_style_set_pad_all(&style_card, 15);
  lv_style_set_border_width(&style_card, 0);

  lv_style_init(&style_btn);
  lv_style_set_bg_color(&style_btn, COLOR_INACTIVE);
  lv_style_set_radius(&style_btn, 8);
  lv_style_set_text_color(&style_btn, lv_color_white());
  lv_style_set_border_width(&style_btn, 0);
  lv_style_set_pad_ver(&style_btn, 14);

  lv_style_init(&style_btn_active);
  lv_style_set_bg_color(&style_btn_active, COLOR_ACCENT);

  lv_style_init(&style_btn_cool);
  lv_style_set_bg_color(&style_btn_cool, COLOR_COOL);
}

// --- Callbacks ---

static void cb_temp_up(lv_event_t *e) {
  setTemperature(getTemp() + 0.5);
  updateUI();
}

static void cb_temp_down(lv_event_t *e) {
  setTemperature(getTemp() - 0.5);
  updateUI();
}

static void cb_power_on(lv_event_t *e) {
  setPower(true);
  updateUI();
}

static void cb_power_off(lv_event_t *e) {
  setPower(false);
  updateUI();
}

static void cb_heat(lv_event_t *e) {
  setMode(true);
  updateUI();
}

static void cb_cool(lv_event_t *e) {
  setMode(false);
  updateUI();
}

static void cb_fan(lv_event_t *e) {
  int speed = (int)(intptr_t)lv_event_get_user_data(e);
  setFanSpeed(speed);
  updateUI();
}

// --- Build UI ---

static lv_obj_t* make_btn(lv_obj_t *parent, const char *text, lv_event_cb_t cb, void *user_data) {
  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_add_style(btn, &style_btn, 0);
  lv_obj_set_flex_grow(btn, 1);
  lv_obj_set_height(btn, 50);
  if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);

  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, text);
  lv_obj_center(lbl);
  return btn;
}

void buildUI() {
  init_styles();

  // Background
  lv_obj_set_style_bg_color(lv_scr_act(), COLOR_BG, 0);

  // Main container — 2 columns
  lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
  lv_obj_set_size(main_cont, 780, 460);
  lv_obj_center(main_cont);
  lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(main_cont, 0, 0);
  lv_obj_set_style_pad_all(main_cont, 0, 0);
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_column(main_cont, 15, 0);

  // === LEFT column: Temperature ===
  lv_obj_t *col_left = lv_obj_create(main_cont);
  lv_obj_set_size(col_left, 370, 450);
  lv_obj_add_style(col_left, &style_card, 0);
  lv_obj_set_flex_flow(col_left, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col_left, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(col_left, 10, 0);

  // Title
  lv_obj_t *title = lv_label_create(col_left);
  lv_label_set_text(title, "VISLA Ventilconvettore");
  lv_obj_set_style_text_color(title, COLOR_ACCENT, 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);

  // Temperature display
  label_temp = lv_label_create(col_left);
  lv_label_set_text(label_temp, "20.5\xC2\xB0""C");
  lv_obj_set_style_text_color(label_temp, COLOR_ACCENT, 0);
  lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_48, 0);

  // Temp buttons row
  lv_obj_t *temp_row = lv_obj_create(col_left);
  lv_obj_set_size(temp_row, 300, 70);
  lv_obj_set_style_bg_opa(temp_row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(temp_row, 0, 0);
  lv_obj_set_style_pad_all(temp_row, 0, 0);
  lv_obj_set_flex_flow(temp_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(temp_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *btn_minus = lv_btn_create(temp_row);
  lv_obj_set_size(btn_minus, 80, 60);
  lv_obj_set_style_radius(btn_minus, 30, 0);
  lv_obj_set_style_bg_color(btn_minus, COLOR_ACCENT, 0);
  lv_obj_add_event_cb(btn_minus, cb_temp_down, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_m = lv_label_create(btn_minus);
  lv_label_set_text(lbl_m, LV_SYMBOL_MINUS);
  lv_obj_set_style_text_font(lbl_m, &lv_font_montserrat_30, 0);
  lv_obj_center(lbl_m);

  lv_obj_t *btn_plus = lv_btn_create(temp_row);
  lv_obj_set_size(btn_plus, 80, 60);
  lv_obj_set_style_radius(btn_plus, 30, 0);
  lv_obj_set_style_bg_color(btn_plus, COLOR_ACCENT, 0);
  lv_obj_add_event_cb(btn_plus, cb_temp_up, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_p = lv_label_create(btn_plus);
  lv_label_set_text(lbl_p, LV_SYMBOL_PLUS);
  lv_obj_set_style_text_font(lbl_p, &lv_font_montserrat_30, 0);
  lv_obj_center(lbl_p);

  // WiFi status
  label_wifi = lv_label_create(col_left);
  lv_label_set_text(label_wifi, "WiFi: ...");
  lv_obj_set_style_text_color(label_wifi, COLOR_DIM, 0);
  lv_obj_set_style_text_font(label_wifi, &lv_font_montserrat_14, 0);

  // Status
  label_status = lv_label_create(col_left);
  lv_label_set_text(label_status, "");
  lv_obj_set_style_text_color(label_status, COLOR_DIM, 0);
  lv_obj_set_style_text_font(label_status, &lv_font_montserrat_12, 0);

  // === RIGHT column: Controls ===
  lv_obj_t *col_right = lv_obj_create(main_cont);
  lv_obj_set_size(col_right, 370, 450);
  lv_obj_set_style_bg_opa(col_right, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(col_right, 0, 0);
  lv_obj_set_style_pad_all(col_right, 0, 0);
  lv_obj_set_flex_flow(col_right, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(col_right, 10, 0);

  // --- Power card ---
  lv_obj_t *card_power = lv_obj_create(col_right);
  lv_obj_set_size(card_power, 370, LV_SIZE_CONTENT);
  lv_obj_add_style(card_power, &style_card, 0);
  lv_obj_set_flex_flow(card_power, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card_power, 8, 0);

  lv_obj_t *lbl_pwr = lv_label_create(card_power);
  lv_label_set_text(lbl_pwr, "Alimentazione");
  lv_obj_set_style_text_color(lbl_pwr, COLOR_DIM, 0);

  lv_obj_t *pwr_row = lv_obj_create(card_power);
  lv_obj_set_size(pwr_row, 330, 55);
  lv_obj_set_style_bg_opa(pwr_row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(pwr_row, 0, 0);
  lv_obj_set_style_pad_all(pwr_row, 0, 0);
  lv_obj_set_flex_flow(pwr_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(pwr_row, 8, 0);

  btn_on  = make_btn(pwr_row, "ACCESO", cb_power_on, NULL);
  btn_off = make_btn(pwr_row, "SPENTO", cb_power_off, NULL);

  // --- Mode card ---
  lv_obj_t *card_mode = lv_obj_create(col_right);
  lv_obj_set_size(card_mode, 370, LV_SIZE_CONTENT);
  lv_obj_add_style(card_mode, &style_card, 0);
  lv_obj_set_flex_flow(card_mode, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card_mode, 8, 0);

  lv_obj_t *lbl_mode = lv_label_create(card_mode);
  lv_label_set_text(lbl_mode, "Stagione");
  lv_obj_set_style_text_color(lbl_mode, COLOR_DIM, 0);

  lv_obj_t *mode_row = lv_obj_create(card_mode);
  lv_obj_set_size(mode_row, 330, 55);
  lv_obj_set_style_bg_opa(mode_row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(mode_row, 0, 0);
  lv_obj_set_style_pad_all(mode_row, 0, 0);
  lv_obj_set_flex_flow(mode_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(mode_row, 8, 0);

  btn_heat = make_btn(mode_row, "CALDO", cb_heat, NULL);
  btn_cool = make_btn(mode_row, "FREDDO", cb_cool, NULL);

  // --- Fan card ---
  lv_obj_t *card_fan = lv_obj_create(col_right);
  lv_obj_set_size(card_fan, 370, LV_SIZE_CONTENT);
  lv_obj_add_style(card_fan, &style_card, 0);
  lv_obj_set_flex_flow(card_fan, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(card_fan, 8, 0);

  lv_obj_t *lbl_fan = lv_label_create(card_fan);
  lv_label_set_text(lbl_fan, "Ventola");
  lv_obj_set_style_text_color(lbl_fan, COLOR_DIM, 0);

  lv_obj_t *fan_row = lv_obj_create(card_fan);
  lv_obj_set_size(fan_row, 330, 55);
  lv_obj_set_style_bg_opa(fan_row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(fan_row, 0, 0);
  lv_obj_set_style_pad_all(fan_row, 0, 0);
  lv_obj_set_flex_flow(fan_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(fan_row, 5, 0);

  const char* fan_labels[] = {"AUTO", "MIN", "NIGHT", "MAX"};
  for (int i = 0; i < 4; i++) {
    btn_fan[i] = make_btn(fan_row, fan_labels[i], cb_fan, (void*)(intptr_t)i);
  }
}

void updateUI() {
  if (!lvgl_port_lock(100)) return;

  // Temperature
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f\xC2\xB0""C", getTemp());
  lv_label_set_text(label_temp, buf);

  // Power buttons
  lv_obj_set_style_bg_color(btn_on,  powerOn ? COLOR_ACCENT : COLOR_INACTIVE, 0);
  lv_obj_set_style_bg_color(btn_off, !powerOn ? COLOR_ACCENT : COLOR_INACTIVE, 0);

  // Mode buttons
  lv_obj_set_style_bg_color(btn_heat, heating ? COLOR_ACCENT : COLOR_INACTIVE, 0);
  lv_obj_set_style_bg_color(btn_cool, !heating ? COLOR_COOL : COLOR_INACTIVE, 0);

  // Fan buttons
  int fs = fanSpeed();
  for (int i = 0; i < 4; i++) {
    lv_obj_set_style_bg_color(btn_fan[i], (i == fs) ? COLOR_ACCENT : COLOR_INACTIVE, 0);
  }

  // Status
  snprintf(buf, sizeof(buf), "R101=0x%04X", regConfig);
  lv_label_set_text(label_status, buf);

  lvgl_port_unlock();
}

// ============================================================
//  Web Server (stesse API di visla-modbus)
// ============================================================

void handleWebRoot() {
  // Redirect to display — the touch IS the main UI
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>VISLA Display</title></head><body style='background:#1a1a2e;color:#eee;font-family:sans-serif;text-align:center;padding:40px'>";
  html += "<h1 style='color:#e94560'>VISLA Ventilconvettore</h1>";
  html += "<p>Il controllo principale e' sul display touch.</p>";
  html += "<p>API: <a href='/api/status' style='color:#e94560'>/api/status</a></p>";
  html += "</body></html>";
  webServer.send(200, "text/html", html);
}

void handleApiStatus() {
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", statusJSON());
}

void handleApiTemperature() {
  if (!webServer.hasArg("value")) { webServer.send(400, "application/json", "{\"error\":\"manca value\"}"); return; }
  float temp = webServer.arg("value").toFloat();
  if (temp < 5.0 || temp > 35.0) { webServer.send(400, "application/json", "{\"error\":\"range 5-35\"}"); return; }
  setTemperature(temp);
  updateUI();
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", statusJSON());
}

void handleApiPower() {
  if (!webServer.hasArg("value")) { webServer.send(400, "application/json", "{\"error\":\"manca value\"}"); return; }
  String val = webServer.arg("value"); val.toLowerCase();
  if (val == "on") { setPower(true); }
  else if (val == "off") { setPower(false); }
  else { webServer.send(400, "application/json", "{\"error\":\"on o off\"}"); return; }
  updateUI();
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", statusJSON());
}

void handleApiFan() {
  if (!webServer.hasArg("value")) { webServer.send(400, "application/json", "{\"error\":\"manca value\"}"); return; }
  String val = webServer.arg("value");
  int speed = -1;
  if (val == "auto" || val == "0") speed = 0;
  else if (val == "min" || val == "1") speed = 1;
  else if (val == "night" || val == "2") speed = 2;
  else if (val == "max" || val == "3") speed = 3;
  if (speed < 0) { webServer.send(400, "application/json", "{\"error\":\"0-3\"}"); return; }
  setFanSpeed(speed);
  updateUI();
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", statusJSON());
}

void handleApiMode() {
  if (!webServer.hasArg("value")) { webServer.send(400, "application/json", "{\"error\":\"manca value\"}"); return; }
  String val = webServer.arg("value"); val.toLowerCase();
  if (val == "heat") { setMode(true); }
  else if (val == "cool") { setMode(false); }
  else { webServer.send(400, "application/json", "{\"error\":\"heat o cool\"}"); return; }
  updateUI();
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", statusJSON());
}

// ============================================================
//  Serial commands (stessi di visla-modbus)
// ============================================================

String serialInput = "";

void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  if (cmd.length() == 0) return;

  if (cmd.startsWith("T")) {
    float temp = cmd.substring(1).toFloat();
    if (temp >= 5.0 && temp <= 35.0) {
      setTemperature(temp);
      Serial.printf(">>> Temperatura: %.1fC\n", temp);
    } else {
      Serial.println("!!! Range 5-35C");
    }
  }
  else if (cmd == "ON")   { setPower(true);  Serial.println(">>> ACCESO"); }
  else if (cmd == "OFF")  { setPower(false); Serial.println(">>> SPENTO"); }
  else if (cmd.startsWith("FAN") && cmd.length() == 4) {
    int s = cmd.charAt(3) - '0';
    if (s >= 0 && s <= 3) { setFanSpeed(s); Serial.printf(">>> Fan: %s\n", fanName()); }
  }
  else if (cmd == "HEAT") { setMode(true);  Serial.println(">>> CALDO"); }
  else if (cmd == "COOL") { setMode(false); Serial.println(">>> FREDDO"); }
  else if (cmd == "STATUS") { Serial.println(statusJSON()); }
  else if (cmd == "SEND")   { sendAllRegisters(); }
  else if (cmd == "IP")     { Serial.println(WiFi.localIP()); }

  updateUI();
}

// ============================================================
//  RS485 bus listener
// ============================================================

#define MAX_FRAME_LEN 256
char frameBuf[MAX_FRAME_LEN];
int fPos = 0;
bool fActive = false;

void processRxByte(char c) {
  if (c == ':') { fPos = 0; fActive = true; }
  else if (c == '\r') {}
  else if (c == '\n' && fActive) {
    frameBuf[fPos] = '\0';
    if (fPos > 0) {
      Serial.printf("[BUS] :%s\n", frameBuf);

#ifdef SNIFFER_MODE
      // Cattura il frame nel buffer
      SniffedFrame frame;
      if (parseModbusFrame(frameBuf, &frame)) {
        sniffBuffer[sniffIndex % SNIFFER_BUFFER_SIZE] = frame;
        sniffIndex++;
      }
#endif
    }
    fActive = false; fPos = 0;
  }
  else if (fActive && fPos < MAX_FRAME_LEN - 1) { frameBuf[fPos++] = c; }
}

// ============================================================
//  WEB PAGES - TEST & SNIFFER
// ============================================================

void handleTest() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>VISLA TEST</title><style>";
  html += "body{font-family:monospace;max-width:500px;margin:10px auto;padding:10px;background:#111;color:#0f0}";
  html += "h2{color:#e94560}button{background:#333;color:#fff;border:1px solid #555;border-radius:6px;padding:10px 12px;font-size:0.9em;cursor:pointer;margin:3px}";
  html += "button:active{background:#e94560}#log{background:#000;padding:10px;border-radius:6px;font-size:0.8em;max-height:200px;overflow-y:auto}";
  html += "</style></head><body><h1 style='color:#e94560'>VISLA - Test Registri S3</h1><div id='log'>Premi un bottone...</div>";
  html += "<h2>REG 101</h2>";
  html += "<button onclick='r(101,0x4003)'>0x4003 FREDDO MAX</button>";
  html += "<button onclick='r(101,0x2003)'>0x2003 CALDO MAX</button>";
  html += "<button onclick='r(101,0x4083)'>0x4083 FREDDO OFF</button>";
  html += "<h2>REG 102</h2>";
  html += "<button onclick='r(102,0x00CD)'>20.5°C</button>";
  html += "<button onclick='r(102,0x0032)'>5.0°C</button>";
  html += "<h2>REG 103</h2>";
  html += "<button onclick='r(103,0x008A)'>0x008A</button>";
  html += "<h2>CUSTOM</h2><input id='reg' type='text' placeholder='101' style='width:60px;padding:5px'>";
  html += "<input id='val' type='text' placeholder='0x4003' style='width:100px;padding:5px'>";
  html += "<button onclick='sendCustom()'>INVIA</button>";
  html += "<script>var logEl=document.getElementById('log');function log(t){logEl.innerHTML=t+'<br>'+logEl.innerHTML}";
  html += "function r(reg,val){log('REG '+reg+' = 0x'+val.toString(16).toUpperCase()+'...');fetch('/api/reg?reg='+reg+'&val='+val).then(r=>r.json()).catch(e=>log('ERR:'+e))}";
  html += "function sendCustom(){var reg=document.getElementById('reg').value;var val=document.getElementById('val').value;r(parseInt(reg),parseInt(val))}";
  html += "</script></body></html>";
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/html", html);
}

void handleSniffer() {
#ifndef SNIFFER_MODE
  webServer.send(403, "text/plain", "SNIFFER MODE non abilitato");
  return;
#endif
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='1'><title>VISLA SNIFFER</title><style>";
  html += "body{font-family:monospace;max-width:900px;margin:10px auto;padding:10px;background:#0a0a0a;color:#0f0}";
  html += "h1{color:#0f0;border-bottom:2px solid #0f0;padding:10px}table{width:100%;border-collapse:collapse}";
  html += "th{background:#1a1a1a;color:#0f0;padding:8px;text-align:left}td{padding:6px;border:1px solid #222}";
  html += "tr:nth-child(odd){background:#151515}.info{background:#1a1a1a;padding:10px;border-radius:4px;margin:10px 0;color:#aaa}";
  html += "</style></head><body><h1>VISLA SNIFFER S3</h1>";
  html += "<div class='info'>Frame catturati: <strong>" + String(sniffIndex) + "</strong> / " + String(SNIFFER_BUFFER_SIZE) + "</div>";
  html += "<button onclick='reset()' style='background:#e74c3c;color:white;padding:10px 20px;border:none;border-radius:6px;cursor:pointer'>RESET</button>";
  html += "<table><tr><th>#</th><th>Data/Ora</th><th>Reg</th><th>Valore</th><th>Azione</th><th>LRC</th></tr>";

  int start = max(0, sniffIndex - SNIFFER_BUFFER_SIZE);
  for (int i = start; i < sniffIndex && i < start + SNIFFER_BUFFER_SIZE; i++) {
    SniffedFrame& f = sniffBuffer[i % SNIFFER_BUFFER_SIZE];
    String lrc = f.lrc_ok ? "OK" : "BAD";
    String key = "act_" + String(i);
    html += "<tr><td>" + String(i+1-start) + "</td><td class='dt' data-ms='" + String(f.timestamp) + "'>--:--:--</td>";
    html += "<td><strong>" + String(f.reg) + "</strong></td><td><strong>0x" + String(f.val, HEX) + "</strong></td>";
    html += "<td><select id='" + key + "' onchange='saveAction(this)' style='padding:4px;font-size:0.85em'>";
    html += "<option value=''>--</option><option value='Accendi'>Accendi</option><option value='Spegni'>Spegni</option>";
    html += "<option value='Temp+'>Temp+</option><option value='Temp-'>Temp-</option></select></td>";
    html += "<td>" + lrc + "</td></tr>";
  }

  html += "</table><script>";
  html += "var pageLoadTime=Date.now();function formatDT(ms){let rows=document.querySelectorAll('tr');if(rows.length<2)return 'xx:xx:xx';";
  html += "let firstMs=parseInt(rows[1].querySelector('.dt').getAttribute('data-ms'));let offset=pageLoadTime-firstMs;";
  html += "let d=new Date(offset+ms);let h=String(d.getHours()).padStart(2,'0');let m=String(d.getMinutes()).padStart(2,'0');";
  html += "let s=String(d.getSeconds()).padStart(2,'0');return h+':'+m+':'+s}";
  html += "function saveAction(sel){sessionStorage.setItem(sel.id,sel.value)}function restoreActions(){";
  html += "document.querySelectorAll('select').forEach(sel=>{let val=sessionStorage.getItem(sel.id);if(val)sel.value=val})}";
  html += "document.querySelectorAll('.dt').forEach(el=>{el.textContent=formatDT(parseInt(el.getAttribute('data-ms')))});";
  html += "restoreActions();function reset(){if(confirm('Reset buffer?')){fetch('/api/reset-sniffer').then(()=>location.reload())}}";
  html += "</script></body></html>";
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/html", html);
}

void handleResetSniffer() {
#ifdef SNIFFER_MODE
  sniffIndex = 0;
  memset(sniffBuffer, 0, sizeof(sniffBuffer));
  webServer.send(200, "application/json", "{\"status\":\"reset\"}");
  Serial.println(">>> Sniffer buffer reset!");
#else
  webServer.send(403, "text/plain", "SNIFFER MODE non abilitato");
#endif
}

void handleApiReg() {
  if (!webServer.hasArg("reg") || !webServer.hasArg("val")) {
    webServer.send(400, "text/plain", "manca reg o val");
    return;
  }
  int reg = webServer.arg("reg").toInt();
  uint16_t val = (uint16_t)strtol(webServer.arg("val").c_str(), NULL, 0);
  if (reg == 101) regConfig = val;
  else if (reg == 103) regMode = val;
  modbusWriteRegister(0, reg, val);
  delay(200);
  webServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ============================================================
//  API COMMANDS - restituisce JSON con ultimi comandi inviati
// ============================================================

void handleApiCommands() {
  String json = "[";
  int count = 0;

  // Calcola quanti comandi abbiamo (max CMD_LOG_SIZE)
  int maxCount = min(cmdLogIndex, CMD_LOG_SIZE);

  // Parte dal primo valido e arriva a cmdLogIndex
  int start = max(0, cmdLogIndex - CMD_LOG_SIZE);

  for (int i = start; i < cmdLogIndex; i++) {
    CommandLog& cmd = cmdLog[i % CMD_LOG_SIZE];
    if (count > 0) json += ",";

    String desc = "";
    if (cmd.reg == 101) {
      desc = (cmd.val & 0x2000) ? "CALDO" : "FREDDO";
      desc += " ";
      int fanSpeed = cmd.val & 0x03;
      if (fanSpeed == 0) desc += "OFF";
      else if (fanSpeed == 1) desc += "MIN";
      else if (fanSpeed == 2) desc += "NIGHT";
      else desc += "MAX";
    } else if (cmd.reg == 102) {
      float temp = cmd.val / 10.0;
      char buf[16];
      snprintf(buf, sizeof(buf), "%.1f°C", temp);
      desc = buf;
    } else if (cmd.reg == 103) {
      desc = "Modo";
    }

    json += "{\"ts\":" + String(cmd.timestamp) +
            ",\"reg\":" + String(cmd.reg) +
            ",\"val\":\"0x" + String(cmd.val, HEX) +
            "\",\"desc\":\"" + desc + "\"}";
    count++;
  }

  json += "]";
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "application/json", json);
}

// ============================================================
//  MONITOR PAGE - pagina web con tabella comandi
// ============================================================

void handleMonitor() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>VISLA MONITOR - Comandi inviati</title>";
  html += "<style>";
  html += "* { margin:0; padding:0; box-sizing:border-box; }";
  html += "body { font-family:monospace; background:#0a0a0a; color:#0f0; padding:20px; }";
  html += "h1 { border-bottom:2px solid #0f0; padding:10px 0; margin:20px 0; }";
  html += ".container { max-width:1000px; margin:0 auto; }";
  html += ".info { background:#1a1a1a; padding:15px; border-radius:4px; margin:15px 0; }";
  html += "table { width:100%; border-collapse:collapse; margin:20px 0; }";
  html += "th { background:#1a1a1a; color:#0f0; padding:12px; text-align:left; border:1px solid #333; font-weight:bold; }";
  html += "td { padding:10px 12px; border:1px solid #222; }";
  html += "tr:nth-child(odd) { background:#0f0a0a; }";
  html += "tr:hover { background:#1a2a1a; }";
  html += ".time { color:#aaa; font-size:0.9em; }";
  html += ".reg { color:#ffff00; font-weight:bold; }";
  html += ".val { color:#00ffff; font-weight:bold; }";
  html += ".desc { color:#00ff00; }";
  html += "button { background:#0f0; color:#000; border:none; padding:10px 20px; border-radius:4px; cursor:pointer; font-weight:bold; font-size:0.95em; }";
  html += "button:hover { background:#0c0; }";
  html += "#count { color:#ffff00; font-size:1.2em; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>VISLA MONITOR - Registri inviati</h1>";
  html += "<div class='info'>";
  html += "Comandi inviati: <span id='count'>0</span> | Aggiornamento: ogni 500ms";
  html += "</div>";
  html += "<button onclick='location.reload()'>🔄 Ricarica</button>";
  html += "<table>";
  html += "<thead>";
  html += "<tr>";
  html += "<th>#</th>";
  html += "<th>Timestamp (ms)</th>";
  html += "<th>Registro</th>";
  html += "<th>Valore</th>";
  html += "<th>Descrizione</th>";
  html += "</tr>";
  html += "</thead>";
  html += "<tbody id='tbody'>";
  html += "</tbody>";
  html += "</table>";
  html += "</div>";
  html += "<script>";
  html += "let lastCount = 0;";
  html += "function loadCommands() {";
  html += "  fetch('/api/commands')";
  html += "    .then(r => r.json())";
  html += "    .then(cmds => {";
  html += "      const tbody = document.getElementById('tbody');";
  html += "      if (cmds.length !== lastCount) {";
  html += "        lastCount = cmds.length;";
  html += "        tbody.innerHTML = '';";
  html += "        cmds.forEach((cmd, idx) => {";
  html += "          const row = document.createElement('tr');";
  html += "          const time = new Date(cmd.ts).toLocaleTimeString('it-IT', {hour:'2-digit',minute:'2-digit',second:'2-digit',fractionalSecondDigits:3});";
  html += "          row.innerHTML = `";
  html += "            <td>${idx + 1}</td>";
  html += "            <td class='time'>${cmd.ts}</td>`;";
  html += "          row.innerHTML += `<td class='reg'>REG ${cmd.reg}</td>`;";
  html += "          row.innerHTML += `<td class='val'>${cmd.val}</td>`;";
  html += "          row.innerHTML += `<td class='desc'>${cmd.desc}</td>`;";
  html += "          tbody.appendChild(row);";
  html += "        });";
  html += "        document.getElementById('count').textContent = cmds.length;";
  html += "      }";
  html += "    })";
  html += "    .catch(e => console.error('Errore caricamento:', e));";
  html += "}";
  html += "loadCommands();";
  html += "setInterval(loadCommands, 500);";
  html += "</script>";
  html += "</body></html>";

  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/html", html);
}

// ============================================================
//  COMMAND SENDER PAGE - pagina per inviare comandi
// ============================================================

void handleCommandSender() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>VISLA COMANDI - Controllo</title>";
  html += "<style>";
  html += "* { margin:0; padding:0; box-sizing:border-box; }";
  html += "body { font-family:monospace; background:#0a0a0a; color:#0f0; padding:20px; }";
  html += "h1 { color:#0f0; border-bottom:2px solid #0f0; padding:10px 0; margin:20px 0; }";
  html += "h2 { color:#ffff00; margin-top:20px; margin-bottom:10px; font-size:1.1em; }";
  html += ".container { max-width:1200px; margin:0 auto; }";
  html += ".section { background:#1a1a1a; padding:20px; border-radius:4px; margin:15px 0; border:1px solid #333; }";
  html += ".button-group { display:flex; gap:10px; flex-wrap:wrap; margin:10px 0; }";
  html += "button { background:#0f0; color:#000; border:none; padding:12px 16px; border-radius:4px; cursor:pointer; font-weight:bold; font-size:0.95em; }";
  html += "button:hover { background:#0c0; }";
  html += "button:active { background:#080; }";
  html += ".btn-danger { background:#f00; }";
  html += ".btn-danger:hover { background:#c00; }";
  html += ".btn-info { background:#0ff; color:#000; }";
  html += ".btn-info:hover { background:#0cc; }";
  html += ".status { padding:12px; border-radius:4px; margin:10px 0; display:none; font-weight:bold; }";
  html += ".status.success { background:#0a3a0a; color:#0f0; border:1px solid #0f0; }";
  html += ".status.error { background:#3a0a0a; color:#f00; border:1px solid #f00; }";
  html += ".info-text { color:#aaa; font-size:0.9em; margin:10px 0; }";
  html += ".input-row { display:flex; gap:10px; align-items:center; margin:10px 0; }";
  html += "input { background:#222; color:#0f0; border:1px solid #333; padding:8px; border-radius:4px; font-family:monospace; }";
  html += "input:focus { outline:none; border-color:#0f0; }";
  html += "label { color:#aaa; min-width:100px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>VISLA COMANDI - Controllo dispositivo</h1>";
  html += "<div id='status' class='status'></div>";

  html += "<div class='section'>";
  html += "<h2>REG 101 - Accensione / Modalità / Ventilatore</h2>";
  html += "<div class='info-text'>Controlla accensione, riscaldamento/raffreddamento e velocità ventilatore</div>";
  html += "<div class='button-group'>";
  html += "<button onclick=\"send(101, 0x4003, 'FREDDO MAX')\">❄️ FREDDO MAX</button>";
  html += "<button onclick=\"send(101, 0x2003, 'CALDO MAX')\">🔥 CALDO MAX</button>";
  html += "<button onclick=\"send(101, 0x4083, 'FREDDO OFF')\" class='btn-danger'>🔌 FREDDO OFF</button>";
  html += "<button onclick=\"send(101, 0x2083, 'CALDO OFF')\" class='btn-danger'>🔌 CALDO OFF</button>";
  html += "</div>";
  html += "</div>";

  html += "<div class='section'>";
  html += "<h2>REG 102 - Temperatura</h2>";
  html += "<div class='info-text'>Imposta la temperatura desiderata (in °C × 10)</div>";
  html += "<div class='button-group'>";
  html += "<button onclick=\"send(102, 0x0080, '12.8°C')\">❄️ 12.8°C</button>";
  html += "<button onclick=\"send(102, 0x00CD, '20.5°C')\">◎ 20.5°C</button>";
  html += "<button onclick=\"send(102, 0x0118, '28.0°C')\">🔥 28.0°C</button>";
  html += "<button onclick=\"send(102, 0x0140, '32.0°C')\">🔥 32.0°C</button>";
  html += "</div>";
  html += "<div class='input-row'>";
  html += "<label>Temperatura:</label>";
  html += "<input type='number' id='tempC' min='5' max='35' value='20.5' step='0.1' style='width:80px'>";
  html += "<span style='color:#aaa'>°C</span>";
  html += "<button onclick=\"sendTemp()\" class='btn-info'>→ INVIA</button>";
  html += "</div>";
  html += "</div>";

  html += "<div class='section'>";
  html += "<h2>REG 103 - Modalità</h2>";
  html += "<div class='info-text'>Configurazione stagionale/operativa</div>";
  html += "<div class='button-group'>";
  html += "<button onclick=\"send(103, 0x008A, 'Modo 0x8A')\">Modo 0x008A</button>";
  html += "<button onclick=\"send(103, 0x00AF, 'Modo 0xAF')\">Modo 0x00AF</button>";
  html += "</div>";
  html += "</div>";

  html += "<div class='section'>";
  html += "<h2>Comando personalizzato</h2>";
  html += "<div class='input-row'>";
  html += "<label>Registro:</label>";
  html += "<input type='number' id='customReg' min='101' max='103' value='101' style='width:80px'>";
  html += "</div>";
  html += "<div class='input-row'>";
  html += "<label>Valore (hex):</label>";
  html += "<input type='text' id='customVal' placeholder='0x4003' value='0x4003' style='width:120px'>";
  html += "<button onclick=\"sendCustom()\" class='btn-info'>→ INVIA</button>";
  html += "</div>";
  html += "</div>";

  html += "</div>";

  html += "<script>";
  html += "function showStatus(msg, isError) {";
  html += "  const el = document.getElementById('status');";
  html += "  el.textContent = msg;";
  html += "  el.className = 'status ' + (isError ? 'error' : 'success');";
  html += "  el.style.display = 'block';";
  html += "  setTimeout(() => el.style.display = 'none', 3000);";
  html += "}";
  html += "function send(reg, val, desc) {";
  html += "  fetch('/api/reg?reg=' + reg + '&val=' + val)";
  html += "    .then(r => r.json())";
  html += "    .then(data => showStatus('✓ REG ' + reg + ' = 0x' + val.toString(16).toUpperCase() + ' (' + desc + ')'))";
  html += "    .catch(e => showStatus('✗ Errore: ' + e, true));";
  html += "}";
  html += "function sendTemp() {";
  html += "  const celsius = parseFloat(document.getElementById('tempC').value);";
  html += "  if (isNaN(celsius) || celsius < 5 || celsius > 35) {";
  html += "    showStatus('✗ Temperatura non valida (5-35°C)', true);";
  html += "    return;";
  html += "  }";
  html += "  const val = Math.round(celsius * 10);";
  html += "  send(102, val, celsius + '°C');";
  html += "}";
  html += "function sendCustom() {";
  html += "  const reg = parseInt(document.getElementById('customReg').value);";
  html += "  const valStr = document.getElementById('customVal').value;";
  html += "  const val = parseInt(valStr, 16);";
  html += "  if (isNaN(reg) || isNaN(val)) {";
  html += "    showStatus('✗ Valori non validi', true);";
  html += "    return;";
  html += "  }";
  html += "  send(reg, val, valStr);";
  html += "}";
  html += "</script>";
  html += "</body></html>";

  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/html", html);
}

// ============================================================
//  SETUP
// ============================================================

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=============================================");
  Serial.println("  VISLA Controller + Display Touch 4.3B");
  Serial.println("  Viessmann Energycal Slim W");
  Serial.println("=============================================\n");

  // === Display + LVGL init ===
  Serial.println("Inizializzazione display...");
  Board *board = new Board();
  board->init();

#if LVGL_PORT_AVOID_TEARING_MODE
  auto lcd = board->getLCD();
  lcd->configFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
#if ESP_PANEL_DRIVERS_BUS_ENABLE_RGB && CONFIG_IDF_TARGET_ESP32S3
  auto lcd_bus = lcd->getBus();
  if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
    static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
  }
#endif
#endif
  assert(board->begin());

  // === Spegni LED rossi PWR/DONE (troppo luminosi) ===
  // I LED sono controllati via CH422G a 0x55 (I2C bus su GPIO8/GPIO9)
  // Registro output: bit 4 (PWR) e bit 5 (DONE)
  Wire.begin(8, 9);  // SDA=GPIO8, SCL=GPIO9
  Wire.beginTransmission(0x55);  // Indirizzo CH422G
  Wire.write(0x01);  // Registro output
  Wire.write(0x00);  // Tutti gli output a LOW (spegni LED)
  Wire.endTransmission();
  Serial.println("LED PWR/DONE spenti via CH422G (I2C 0x55)");

  Serial.println("Inizializzazione LVGL...");
  lvgl_port_init(board->getLCD(), board->getTouch());

  lvgl_port_lock(-1);
  buildUI();
  updateUI();
  lvgl_port_unlock();
  Serial.println("UI pronta!");

  // === RS485 init ===
  Serial.println("Inizializzazione RS485...");
  RS485.begin(BAUD_RATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  delay(200);
  Serial.println("RS485 OK (TX=44, RX=43)");

  // === WiFi ===
  Serial.printf("Connessione WiFi: %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connesso! IP: %s\n", WiFi.localIP().toString().c_str());

    webServer.on("/", handleWebRoot);
    webServer.on("/api/status", HTTP_GET, handleApiStatus);
    webServer.on("/api/temperature", handleApiTemperature);
    webServer.on("/api/power", handleApiPower);
    webServer.on("/api/fan", handleApiFan);
    webServer.on("/api/mode", handleApiMode);
    webServer.on("/api/reg", HTTP_GET, handleApiReg);
    webServer.on("/api/commands", HTTP_GET, handleApiCommands);
    webServer.on("/monitor", HTTP_GET, handleMonitor);
    webServer.on("/sender", HTTP_GET, handleCommandSender);
    webServer.on("/test", handleTest);
    webServer.on("/sniffer", handleSniffer);
    webServer.on("/api/reset-sniffer", HTTP_GET, handleResetSniffer);
    webServer.begin();
    Serial.println("Web server avviato sulla porta 80");

    // === OTA (Over-The-Air) Update ===
    ArduinoOTA.setHostname("viessmann-controller");
    ArduinoOTA.onStart([]() {
      Serial.println("\n>>> OTA UPDATE STARTED <<<");
      Serial.println("Spegnimento servizi...");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\n>>> OTA UPDATE COMPLETE <<<");
      Serial.println("Riavvio in corso...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA Progress: %u/%u (%u%%)\r", progress, total, (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Error: %u - ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA avviato! Puoi aggiornare con:");
    Serial.printf("  platformio run -e esp32s3-43b -t upload --upload-port=%s\n", WiFi.localIP().toString().c_str());

    // Update WiFi label on display
    if (lvgl_port_lock(100)) {
      char wbuf[64];
      snprintf(wbuf, sizeof(wbuf), "WiFi: %s  IP: %s", WIFI_SSID, WiFi.localIP().toString().c_str());
      lv_label_set_text(label_wifi, wbuf);
      lvgl_port_unlock();
    }
  } else {
    Serial.println("\nWiFi non connesso.");
    if (lvgl_port_lock(100)) {
      lv_label_set_text(label_wifi, "WiFi: non connesso");
      lvgl_port_unlock();
    }
  }

  // Primo invio
  sendAllRegisters();
  lastSend = millis();
}

// ============================================================
//  LOOP
// ============================================================

void loop() {
  // OTA Update handler (controlla continuamente se c'è un aggiornamento)
  ArduinoOTA.handle();

  // Web server
  webServer.handleClient();

  // Serial USB
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (serialInput.length() > 0) {
        processCommand(serialInput);
        serialInput = "";
      }
    } else {
      serialInput += c;
    }
  }

  // RS485 bus
  while (RS485.available()) {
    uint8_t raw = RS485.read();
    processRxByte((char)(raw & 0x7F));
  }

  // Temperature debounce (aspetta 1 sec, poi invia)
  if (tempPending && (millis() - tempChangeTime >= TEMP_DEBOUNCE_MS)) {
    tempPending = false;
    regTemp = (uint16_t)(pendingTemp * 10);
    Serial.printf("[TEMP] Debounce scaduto, invio temperatura: %.1f°C\n", pendingTemp);
    sendAllRegisters();
  }

  // Invio periodico (keep-alive ogni 68 sec, anche quando spento - come master)
  if (millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();
    sendAllRegisters();
  }

  delay(5);  // yield for LVGL task
}
