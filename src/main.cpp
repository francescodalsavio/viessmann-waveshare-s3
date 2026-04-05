/*
 * VISLA Modbus Display — ESP32-S3-Touch-LCD-4.3B
 * Viessmann Energycal Slim W — Controllo con display touch
 *
 * Controllo via:
 *   1. Display touch LVGL 800x480
 *   2. API REST via WiFi (http://<ip>/api/...)
 *   3. Seriale USB (comandi testuali)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_display_panel.hpp>
#include <lvgl.h>
#include "lvgl_v8_port.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

// === WiFi Config ===
const char* WIFI_SSID = "Molinella";
const char* WIFI_PASS = "Fastweb10";

// === RS485 Pin Definitions (Waveshare ESP32-S3-Touch-LCD-4.3B) ===
#define RS485_TX_PIN  44
#define RS485_RX_PIN  43
#define RS485          Serial1
#define BAUD_RATE      9600
#define SEND_INTERVAL  10000  // 10 sec

// === Stato ventilconvettore ===
uint16_t regConfig = 0x2003;  // caldo acceso, ventola MAX
uint16_t regTemp   = 0x00CD;  // 20.5C (x10)
uint16_t regMode   = 0x008A;  // modo stagionale
bool     powerOn   = true;
bool     heating   = true;

// === Web Server ===
WebServer webServer(80);

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

  while (RS485.available()) RS485.read();
  RS485.write((uint8_t*)txBuf, pos);
  RS485.flush();
}

void sendAllRegisters() {
  Serial.println(">>> Invio registri...");
  modbusWriteRegister(0, 101, regConfig);
  delay(200);
  modbusWriteRegister(0, 102, regTemp);
  delay(200);
  modbusWriteRegister(0, 103, regMode);
  Serial.printf("    101=0x%04X 102=0x%04X(%.1fC) 103=0x%04X OK\n",
                regConfig, regTemp, regTemp / 10.0, regMode);
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
  if (on) {
    regConfig &= ~((1 << 14) | (1 << 13) | (1 << 7));
    if (heating) regConfig |= (1 << 13);
    else         regConfig |= (1 << 14);
    powerOn = true;
  } else {
    regConfig |= (1 << 7);
    powerOn = false;
  }
  sendAllRegisters();
}

void setFanSpeed(int speed) {
  if (speed < 0 || speed > 3) return;
  regConfig = (regConfig & ~0x03) | (speed & 0x03);
  sendAllRegisters();
}

void setMode(bool heat) {
  heating = heat;
  if (heat) {
    regMode |= 0x02;
    if (powerOn) { regConfig &= ~(1 << 14); regConfig |= (1 << 13); }
  } else {
    regMode &= ~0x02;
    if (powerOn) { regConfig &= ~(1 << 13); regConfig |= (1 << 14); }
  }
  sendAllRegisters();
}

void setTemperature(float temp) {
  if (temp < 5.0) temp = 5.0;
  if (temp > 35.0) temp = 35.0;
  regTemp = (uint16_t)(temp * 10);
  sendAllRegisters();
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
    if (fPos > 0) Serial.printf("[BUS] :%s\n", frameBuf);
    fActive = false; fPos = 0;
  }
  else if (fActive && fPos < MAX_FRAME_LEN - 1) { frameBuf[fPos++] = c; }
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
    webServer.begin();
    Serial.println("Web server avviato sulla porta 80");

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

  // Invio periodico
  if (powerOn && millis() - lastSend >= SEND_INTERVAL) {
    lastSend = millis();
    sendAllRegisters();
  }

  delay(5);  // yield for LVGL task
}
