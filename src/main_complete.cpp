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

// ========== ASCII PARSER (MODBUS ASCII) ==========

void captureRS485Frame() {
  // Parser ASCII MODBUS (formato: :AAFFRRRRRRRRLLCC\r\n)
  static String rxBuffer = "";

  if (Serial1.available()) {
    char c = Serial1.read();

    if (c == ':') {
      // Inizio nuovo frame
      rxBuffer = "";
    } else if (c == '\r' || c == '\n') {
      // Fine frame
      if (rxBuffer.length() > 0) {
        SniffedFrame frame;
        frame.timestamp = millis();
        strcpy(frame.rawHex, rxBuffer.c_str());

        // Parse il frame ASCII
        // Formato: AAFFRRRRRRRRLLCC
        if (rxBuffer.length() == 16) {
          // Extract bytes
          uint8_t bytes[8];
          for (int i = 0; i < 8; i++) {
            char hex[3] = {rxBuffer[i*2], rxBuffer[i*2+1], 0};
            bytes[i] = (uint8_t)strtol(hex, NULL, 16);
          }

          frame.addr = bytes[0];
          frame.func = bytes[1];
          frame.reg = (bytes[2] << 8) | bytes[3];
          frame.val = (bytes[4] << 8) | bytes[5];
          frame.lrc = bytes[6];
          frame.lrc_ok = true;  // TODO: verify LRC if needed

          addToSnifferBuffer(frame);

          Serial.printf("[SNIFFER] ✓ ASCII: Addr=0x%02X Func=%d Reg=0x%04X(#%d) Val=0x%04X LRC=0x%02X\n",
                       frame.addr, frame.func, frame.reg, frame.reg - 0x0064, frame.val, frame.lrc);
        }
        rxBuffer = "";
      }
    } else if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
      // Accumula hex characters
      rxBuffer += (char)toupper(c);
    }
  }
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
  // Simple HTML response to test if web server is working
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='utf-8'>";
  html += "<title>Sniffer</title>";
  html += "<style>body { font-family: monospace; background: #1e1e1e; color: #00ff00; }</style>";
  html += "</head><body>";
  html += "<h1>RS485 Sniffer</h1>";
  html += "<p>Total Frames: " + String(snifferCount) + "</p>";
  html += "<table border='1' style='border-collapse:collapse'>";
  html += "<tr><th>#</th><th>REG</th><th>HEX</th><th>Decimal</th><th>Delay(ms)</th></tr>";
  
  // Mostra ultimi 20 frame
  int start = (snifferCount < SNIFFER_BUFFER_SIZE) ? 0 : snifferIndex;
  int count = (snifferCount < SNIFFER_BUFFER_SIZE) ? snifferCount : SNIFFER_BUFFER_SIZE;
  int display_count = (count > 20) ? 20 : count;
  
  for (int i = count - display_count; i < count; i++) {
    if (i < 0) continue;
    int idx = (start + i) % SNIFFER_BUFFER_SIZE;
    SniffedFrame &f = snifferBuffer[idx];
    int regNum = f.reg - 0x0064;
    uint32_t delay = (i > 0) ? (f.timestamp - snifferBuffer[(start + i - 1) % SNIFFER_BUFFER_SIZE].timestamp) : 0;
    
    html += "<tr>";
    html += "<td>" + String(snifferCount - count + i + 1) + "</td>";
    html += "<td>" + String(regNum) + "</td>";
    html += "<td>0x" + String(f.val, HEX) + "</td>";
    html += "<td>" + String(f.val) + "</td>";
    html += "<td>" + String(delay) + "</td>";
    html += "</tr>";
  }
  
  html += "</table>";
  html += "<p><a href='/sniffer'>Refresh</a></p>";
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
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
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
  
  // Update IP on display
  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    snifferView.setIP(ip.c_str());
  } else {
    snifferView.setIP("Not connected");
  }
  
  lvgl_port_unlock();

  // Skip sending commands in SNIFFER MODE
  // model.setPower(true);

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

  // UI update disabled for now - web server priority!
  // if (millis() - lastSnifferUpdate > 500) {
  //   int start = (snifferCount < SNIFFER_BUFFER_SIZE) ? 0 : snifferIndex;
  //   int count = (snifferCount < SNIFFER_BUFFER_SIZE) ? snifferCount : SNIFFER_BUFFER_SIZE;
  //   snifferView.updateFrames(snifferBuffer, count, snifferCount, 0xFFFFFFFF, 0, 0);
  //   lastSnifferUpdate = millis();
  // }

  if (millis() - lastLoopPrint > 5000) {
    Serial.printf("[LOOP] Frames captured: %d | Serial1.available()=%d\n", snifferCount, Serial1.available());
    lastLoopPrint = millis();
  }

  delay(2);  // Ridotto da 5 a 2ms
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
