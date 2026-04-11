/*
 * ═══════════════════════════════════════════════════════════════
 * VISLA — CLEAN ARCHITECTURE COMPLETE v4.0
 * ═══════════════════════════════════════════════════════════════
 *
 * Versione FINALE con Dependency Inversion Pattern
 *
 * Architettura Pulita:
 *
 *                    ┌─────────────────────┐
 *                    │  View (LVGL)        │
 *                    └────────────┬────────┘
 *                                 │
 *                    ┌────────────▼────────┐
 *                    │  ViewModel          │
 *                    └────────────┬────────┘
 *                                 │
 *                    ┌────────────▼────────┐
 *                    │  Use Cases          │
 *                    │  (dipendono da)     │
 *                    │  ↓                  │
 *                    │  IRepository ◄──────┼─── Abstract Interface
 *                    └────────────┬────────┘
 *                                 │
 *                                 │ (implementa)
 *                    ┌────────────▼────────┐
 *                    │  RepositoryImpl      │
 *                    │  (concrete)         │
 *                    └────────────┬────────┘
 *                                 │
 *                    ┌────────────▼────────┐
 *                    │  ModbusService      │
 *                    └─────────────────────┘
 *
 * KEY POINT: Use Cases conoscono SOLO IViessmannRepository (interface)
 *            Non conoscono ViessmannRepositoryImpl (implementazione)
 *            Questo è Dependency Inversion!
 */

#define SNIFFER_MODE

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <lvgl.h>
#include "lvgl_v8_port.h"
#include "esp_display_panel.hpp"

using namespace esp_panel::board;
using namespace esp_panel::drivers;

// ========== LAYER 0: DEVICES ==========
#include "model/modbus_service.h"

// ========== LAYER 1: DATA ACCESS ==========
#include "domain/i_viessmann_repository.h"      // ← Abstract Interface (in Domain!)
#include "data/viessmann_repository_impl.h"     // ← Concrete Implementation (in Data)

// ========== LAYER 2: DOMAIN (USE CASES) ==========
#include "domain/use_cases_v2.h"                // ← Dipende da IViessmannRepository

// ========== LAYER 3: MODEL ==========
#include "model/viessmann_model_v2.h"

// ========== LAYER 4: PRESENTATION ==========
#include "viewmodel/viessmann_viewmodel.h"
#include "view/viessmann_view.h"
#include "view/sniffer_view.h"  // Sniffer UI per RS485

// ========== CONFIG ==========
const char* WIFI_SSID = "Molinella";
const char* WIFI_PASS = "Fastweb10";

// ========== GLOBAL INSTANCES (Dependency Injection) ==========

// Layer 0: Devices
ModbusService modbus;

// Layer 1: Data (Concrete Implementation)
ViessmannRepositoryImpl repository(modbus);

// Layer 3: Model
ViessmannModel model;

// Layer 2: Use Cases (dipendono da IViessmannRepository interface, NON dall'impl!)
SetTemperatureUseCase setTempUC(model, (IViessmannRepository&)repository);
TogglePowerUseCase togglePowerUC(model, (IViessmannRepository&)repository);
ChangeHeatingModeUseCase changeModeUC(model, (IViessmannRepository&)repository);
ChangeFanSpeedUseCase changeFanUC(model, (IViessmannRepository&)repository);

// Layer 4: Presentation
ViessmannViewModel viewModel(model);
ViessmannView view(viewModel);
SnifferView snifferView;  // UI per sniffer

// Web Server
WebServer webServer(80);

// ========== SNIFFER BUFFER (per catturare frame da master) ==========
// SniffedFrame è definita in sniffer_view.h (include in main_complete.cpp)
#define SNIFFER_BUFFER_SIZE 100

SniffedFrame snifferBuffer[SNIFFER_BUFFER_SIZE];
int snifferIndex = 0;
uint32_t snifferCount = 0;

void addToSnifferBuffer(const SniffedFrame &frame) {
  snifferBuffer[snifferIndex] = frame;
  snifferIndex = (snifferIndex + 1) % SNIFFER_BUFFER_SIZE;
  snifferCount++;
}

void captureRS485Frame() {
  // Leggi dati in arrivo da RS485 (master che invia)
#ifdef ARDUINO
  static String rxBuffer = "";

  if (Serial1.available()) {
    char c = Serial1.read();

    if (c == ':') {
      rxBuffer = "";  // Nuovo frame inizia
    } else if (c == '\r' || c == '\n') {
      if (rxBuffer.length() > 0) {
        // Completo frame ricevuto
        SniffedFrame frame;
        frame.timestamp = millis();
        strcpy(frame.rawHex, rxBuffer.c_str());

        // Parse il frame
        ModbusFrame mf;
        if (modbus.parseFrame(rxBuffer.c_str(), &mf)) {
          frame.addr = mf.addr;
          frame.func = mf.func;
          frame.reg = mf.reg;
          frame.val = mf.val;
          frame.lrc = mf.lrc;
          frame.lrc_ok = mf.lrc_ok;

          addToSnifferBuffer(frame);

          Serial.printf("[SNIFFER] Frame RX: REG=0x%04X(#%d) VAL=0x%04X LRC=%s\n",
                       frame.reg, frame.reg - 0x0064,
                       frame.val, frame.lrc_ok ? "OK" : "BAD");
        }
        rxBuffer = "";
      }
    } else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      rxBuffer += c;
    }
  }
#endif
}

// ========== WEB API ==========

void handleStatus() {
  String json = "{";
  json += "\"power\":\"" + String(model.isPowerOn() ? "on" : "off") + "\",";
  json += "\"temperature\":" + String(model.getTemperature(), 1) + ",";
  json += "\"mode\":\"" + String(model.isHeating() ? "heat" : "cool") + "\",";
  json += "\"fan_speed\":" + String((int)model.getFanSpeed()) + ",";
  json += "\"version\":\"Clean Architecture v4.0 Complete\"";
  json += "}";
  webServer.send(200, "application/json", json);
}

void handlePowerOn() {
  model.setPower(true);
  webServer.send(200, "text/plain", "Power ON");
}

void handlePowerOff() {
  model.setPower(false);
  webServer.send(200, "text/plain", "Power OFF");
}

void handleSetTemperature() {
  if (webServer.hasArg("value")) {
    float temp = webServer.arg("value").toFloat();
    model.setTemperature(temp);
    webServer.send(200, "text/plain", "Temperature set");
  } else {
    webServer.send(400, "text/plain", "Missing 'value' parameter");
  }
}

void handleSetMode() {
  if (webServer.hasArg("mode")) {
    String mode = webServer.arg("mode");
    model.setHeating(mode == "heat");
    webServer.send(200, "text/plain", mode == "heat" ? "Heating" : "Cooling");
  } else {
    webServer.send(400, "text/plain", "Missing 'mode' parameter");
  }
}

void handleSetFanSpeed() {
  if (webServer.hasArg("speed")) {
    int speed = webServer.arg("speed").toInt();
    model.setFanSpeed((ViessmannModel::FanSpeed)speed);
    webServer.send(200, "text/plain", "Fan speed set");
  } else {
    webServer.send(400, "text/plain", "Missing 'speed' parameter");
  }
}

void handleSniffer() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Modbus Sniffer</title>";
  html += "<style>";
  html += "body { font-family: monospace; margin: 10px; background: #1e1e1e; color: #00ff00; }";
  html += "h1, h2 { border-bottom: 2px solid #00ff00; padding-bottom: 10px; }";
  html += "table { border-collapse: collapse; width: 100%; margin-top: 20px; }";
  html += "th, td { border: 1px solid #00ff00; padding: 8px; text-align: left; font-size: 12px; }";
  html += "th { background: #003300; font-weight: bold; }";
  html += "tr:nth-child(even) { background: #0a0a0a; }";
  html += ".ok { color: #00ff00; } .bad { color: #ff0000; }";
  html += ".info { margin: 10px 0; padding: 10px; background: #0a0a0a; border-left: 3px solid #00ff00; }";
  html += ".hex { font-weight: bold; color: #ffff00; }";
  html += ".delay { color: #ff9900; }";
  html += ".stats { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-top: 20px; }";
  html += ".stat-box { border: 1px solid #00ff00; padding: 15px; background: #0a0a0a; }";
  html += ".stat-box h3 { margin: 0 0 10px 0; }";
  html += ".stat-item { padding: 5px 0; border-bottom: 1px solid #333; }";
  html += ".stat-item:last-child { border-bottom: none; }";
  html += "</style>";
  html += "<script>";
  html += "function refresh() { location.reload(); }";
  html += "setInterval(refresh, 2000);";
  html += "</script>";
  html += "</head><body>";

  html += "<h1>📡 Modbus RS485 Sniffer Dashboard</h1>";
  html += "<div class='info'>";
  html += "<b>Total Frames:</b> " + String(snifferCount) + " | ";
  html += "<b>Buffer:</b> " + String(snifferCount > SNIFFER_BUFFER_SIZE ? SNIFFER_BUFFER_SIZE : snifferCount) + "/" + String(SNIFFER_BUFFER_SIZE) + " | ";
  html += "<b>Auto-refresh:</b> 2s<br>";
  html += "</div>";

  // Calcola statistiche
  int start = (snifferCount < SNIFFER_BUFFER_SIZE) ? 0 : snifferIndex;
  int count = (snifferCount < SNIFFER_BUFFER_SIZE) ? snifferCount : SNIFFER_BUFFER_SIZE;

  // Conta occorrenze per registro
  int reg101_count = 0, reg102_count = 0, reg103_count = 0;
  uint32_t prev_timestamp = 0;
  uint32_t min_delay = 0xFFFFFFFF, max_delay = 0, avg_delay = 0, delay_sum = 0, delay_count = 0;

  for (int i = 0; i < count; i++) {
    int idx = (start + i) % SNIFFER_BUFFER_SIZE;
    SniffedFrame &frame = snifferBuffer[idx];

    if (frame.reg == 0x0065) reg101_count++;
    if (frame.reg == 0x0066) reg102_count++;
    if (frame.reg == 0x0067) reg103_count++;

    if (prev_timestamp > 0) {
      uint32_t delay = frame.timestamp - prev_timestamp;
      if (delay > 0 && delay < 10000) {  // Ignora delay strani
        if (delay < min_delay) min_delay = delay;
        if (delay > max_delay) max_delay = delay;
        delay_sum += delay;
        delay_count++;
      }
    }
    prev_timestamp = frame.timestamp;
  }
  if (delay_count > 0) avg_delay = delay_sum / delay_count;

  // Statistiche
  html += "<div class='stats'>";
  html += "<div class='stat-box'>";
  html += "<h3>📊 Command Pattern</h3>";
  html += "<div class='stat-item'>REG 101 (Config): " + String(reg101_count) + " sends</div>";
  html += "<div class='stat-item'>REG 102 (Temp): " + String(reg102_count) + " sends</div>";
  html += "<div class='stat-item'>REG 103 (Mode): " + String(reg103_count) + " sends</div>";
  html += "</div>";
  html += "<div class='stat-box'>";
  html += "<h3>⏱️ Timing Analysis</h3>";
  if (delay_count > 0) {
    html += "<div class='stat-item'>Min delay: <span class='delay'>" + String(min_delay) + " ms</span></div>";
    html += "<div class='stat-item'>Max delay: <span class='delay'>" + String(max_delay) + " ms</span></div>";
    html += "<div class='stat-item'>Avg delay: <span class='delay'>" + String(avg_delay) + " ms</span></div>";
    html += "<div class='stat-item'>Total delays sampled: " + String(delay_count) + "</div>";
  } else {
    html += "<div class='stat-item'>Not enough data</div>";
  }
  html += "</div>";
  html += "</div>";

  html += "<h2>Frame Details</h2>";
  html += "<table>";
  html += "<tr><th>#</th><th>Delay (ms)</th><th>Timestamp</th><th>Addr</th><th>Func</th><th>Register</th><th>REG#</th><th>Value (Hex)</th><th>Value (Dec)</th><th>Description</th><th>LRC</th></tr>";

  prev_timestamp = 0;
  for (int i = 0; i < count; i++) {
    int idx = (start + i) % SNIFFER_BUFFER_SIZE;
    SniffedFrame &frame = snifferBuffer[idx];

    int regNum = frame.reg - 0x0064;
    String regName = "";
    switch (frame.reg) {
      case 0x0065: regName = "Config"; break;
      case 0x0066: regName = "Temperature"; break;
      case 0x0067: regName = "Mode"; break;
      default: regName = "Unknown"; break;
    }

    uint32_t delay = 0;
    if (prev_timestamp > 0) {
      delay = frame.timestamp - prev_timestamp;
    }
    prev_timestamp = frame.timestamp;

    html += "<tr>";
    html += "<td>" + String(snifferCount - count + i + 1) + "</td>";
    html += "<td class='delay'>" + String(delay) + "</td>";
    html += "<td>" + String(frame.timestamp) + "</td>";
    html += "<td class='hex'>0x" + String(frame.addr, HEX) + "</td>";
    html += "<td class='hex'>0x" + String(frame.func, HEX) + "</td>";
    html += "<td class='hex'>0x" + String(frame.reg, HEX) + "</td>";
    html += "<td>" + String(regNum) + "</td>";
    html += "<td class='hex'>0x" + String(frame.val, HEX) + "</td>";
    html += "<td>" + String(frame.val);
    if (frame.reg == 0x0066) {
      float celsius = frame.val / 10.0f;
      html += " (🌡️ " + String(celsius, 1) + "°C)";
    }
    html += "</td>";
    html += "<td>" + regName;
    if (frame.reg == 0x0065) {
      int bit14 = (frame.val >> 14) & 1;
      int bit13 = (frame.val >> 13) & 1;
      int bit7 = (frame.val >> 7) & 1;
      int fan = frame.val & 0x03;
      html += " [";
      html += (bit14 ? "COOL" : "");
      html += (bit13 ? "HEAT" : "");
      if (bit14 == 0 && bit13 == 0) html += "NONE";
      html += " ";
      html += (bit7 ? "STANDBY" : "ON");
      html += " FAN:" + String(fan) + "]";
    }
    html += "</td>";
    html += "<td class='" + String(frame.lrc_ok ? "ok" : "bad") + "'>";
    html += (frame.lrc_ok ? "✓" : "✗");
    html += "</td>";
    html += "</tr>";
  }

  html += "</table>";

  html += "<div class='info' style='margin-top: 20px;'>";
  html += "<h2>Register Reference</h2>";
  html += "<b>REG 101 (0x0065):</b> Config register<br>";
  html += "  • Bit 14: FREDDO (Cooling) = 1, else CALDO (Heating)<br>";
  html += "  • Bit 13: CALDO (Heating) = 1, else FREDDO<br>";
  html += "  • Bit 7: STANDBY = 1, ON = 0<br>";
  html += "  • Bit 1-0: FAN speed (0=OFF, 1=MIN, 2=AUTO, 3=MAX)<br>";
  html += "<br>";
  html += "<b>REG 102 (0x0066):</b> Temperature setpoint<br>";
  html += "  • Value = Celsius × 10 (e.g., 0xCD = 205 = 20.5°C)<br>";
  html += "  • Range: 0x32-0xF0 (5.0-24.0°C)<br>";
  html += "<br>";
  html += "<b>REG 103 (0x0067):</b> Mode register (usually 0xB9 = seasonal)<br>";
  html += "</div>";

  html += "</body></html>";

  webServer.send(200, "text/html", html);
}

void setupWebServer() {
  webServer.on("/status", HTTP_GET, handleStatus);
  webServer.on("/power/on", HTTP_POST, handlePowerOn);
  webServer.on("/power/off", HTTP_POST, handlePowerOff);
  webServer.on("/temperature", HTTP_POST, handleSetTemperature);
  webServer.on("/mode", HTTP_POST, handleSetMode);
  webServer.on("/fan", HTTP_POST, handleSetFanSpeed);
  webServer.on("/sniffer", HTTP_GET, handleSniffer);
  webServer.begin();
  Serial.println("[BOOT] Web Server started");
  Serial.println("[BOOT]   -> /status (GET) — JSON status");
  Serial.println("[BOOT]   -> /sniffer (GET) — Modbus RS485 sniffer dashboard");
}

void setupWiFi() {
  Serial.print("[BOOT] Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[BOOT] WiFi Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[BOOT] WiFi Failed");
  }
}

static uint32_t millis_cb(void) {
  return millis();
}

// ========== SETUP ==========

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n╔═════════════════════════════════════════╗");
  Serial.println("║  VISLA Clean Architecture v4.0 COMPLETE  ║");
  Serial.println("║  Dependency Inversion Pattern             ║");
  Serial.println("╚═════════════════════════════════════════╝\n");

  // Inject Use Cases nel Model
  Serial.println("[BOOT] Injecting Use Cases...");
  model.injectUseCases(&setTempUC, &togglePowerUC, &changeModeUC, &changeFanUC);

  // Init
  Serial.println("[BOOT] Initializing Modbus...");
  modbus.begin();

  Serial.println("[BOOT] Initializing WiFi...");
  setupWiFi();

  Serial.println("[BOOT] Initializing Web Server...");
  setupWebServer();

  Serial.println("[BOOT] Initializing Display Panel...");
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
  board->begin();

  Serial.println("[BOOT] Initializing LVGL...");
  lv_init();
  lvgl_port_init(board->getLCD(), board->getTouch());

  Serial.println("[BOOT] Creating Sniffer UI...");
  lvgl_port_lock(-1);
  snifferView.create();
  snifferView.show();
  lvgl_port_unlock();

  Serial.println("[BOOT] Sending initial state...");
  model.setPower(true);

  Serial.println("\n[BOOT] Setup complete! ✓\n");
}

// ========== LOOP ==========

uint32_t lastLoopPrint = 0;
uint32_t lastSnifferUpdate = 0;

void loop() {
  // Cattura frame Modbus dal master (SNIFFER MODE)
  captureRS485Frame();

  webServer.handleClient();
  lv_timer_handler();

  // Aggiorna UI sniffer ogni 500ms
  if (millis() - lastSnifferUpdate > 500) {
    // Calcola statistiche
    int start = (snifferCount < SNIFFER_BUFFER_SIZE) ? 0 : snifferIndex;
    int count = (snifferCount < SNIFFER_BUFFER_SIZE) ? snifferCount : SNIFFER_BUFFER_SIZE;

    uint32_t min_delay = 0xFFFFFFFF, max_delay = 0, avg_delay = 0, delay_sum = 0, delay_count = 0;
    uint32_t prev_timestamp = 0;

    for (int i = 0; i < count; i++) {
      int idx = (start + i) % SNIFFER_BUFFER_SIZE;
      SniffedFrame &frame = snifferBuffer[idx];

      if (prev_timestamp > 0) {
        uint32_t delay = frame.timestamp - prev_timestamp;
        if (delay > 0 && delay < 10000) {
          if (delay < min_delay) min_delay = delay;
          if (delay > max_delay) max_delay = delay;
          delay_sum += delay;
          delay_count++;
        }
      }
      prev_timestamp = frame.timestamp;
    }
    if (delay_count > 0) avg_delay = delay_sum / delay_count;

    // Update display
    snifferView.updateFrames(snifferBuffer, count, snifferCount,
                            delay_count > 0 ? min_delay : 0xFFFFFFFF,
                            max_delay, avg_delay);

    lastSnifferUpdate = millis();
  }

  if (millis() - lastLoopPrint > 10000) {
    Serial.printf("[LOOP] Frames captured: %d | Display updating\n", snifferCount);
    lastLoopPrint = millis();
  }

  delay(5);
}

/*
 * =================================================================
 * COSA FA QUESTA VERSIONE:
 * =================================================================
 *
 * OK DEPENDENCY INVERSION (SOLID - D)
 *    Use Cases non conoscono ViessmannRepositoryImpl
 *    Dipendono SOLO da IViessmannRepository (abstract interface)
 *
 * OK TESTABILITA COMPLETA
 *    Puoi creare MockRepository che implementa IViessmannRepository
 *    Puoi testare Use Cases senza hardware
 *
 * OK SWAPPABLE IMPLEMENTATION
 *    Domani se cambi da Modbus a WiFi:
 *    - Crei ViessmannRepositoryWiFi : public IViessmannRepository
 *    - Cambi SOLO la instantiazione nel main
 *    - Use Cases NON cambiano!
 *
 * OK TRUE SEPARATION OF CONCERNS
 *    Domain (Use Cases) = Business Logic pura
 *    Data (Repository) = Come raggiungere i dati
 *    Model = Stato
 *    ViewModel = Presentazione
 *    View = UI
 *
 * =================================================================
 */
