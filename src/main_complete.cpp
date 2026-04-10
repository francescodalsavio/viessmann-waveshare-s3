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

// Web Server
WebServer webServer(80);

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

void setupWebServer() {
  webServer.on("/status", HTTP_GET, handleStatus);
  webServer.on("/power/on", HTTP_POST, handlePowerOn);
  webServer.on("/power/off", HTTP_POST, handlePowerOff);
  webServer.on("/temperature", HTTP_POST, handleSetTemperature);
  webServer.on("/mode", HTTP_POST, handleSetMode);
  webServer.on("/fan", HTTP_POST, handleSetFanSpeed);
  webServer.begin();
  Serial.println("[BOOT] Web Server started");
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

  Serial.println("[BOOT] Initializing LVGL...");
  lv_init();
  lv_tick_set_cb(millis_cb);

  Serial.println("[BOOT] Creating UI...");
  view.create();

  Serial.println("[BOOT] Sending initial state...");
  model.setPower(true);

  Serial.println("\n[BOOT] Setup complete! ✓\n");
}

// ========== LOOP ==========

uint32_t lastLoopPrint = 0;

void loop() {
  webServer.handleClient();
  lv_timer_handler();

  if (millis() - lastLoopPrint > 10000) {
    Serial.printf("[LOOP] Temp=%.1f°C Power=%s Mode=%s Fan=%d\n",
                  model.getTemperature(),
                  model.isPowerOn() ? "ON" : "OFF",
                  model.isHeating() ? "HEAT" : "COOL",
                  (int)model.getFanSpeed());
    lastLoopPrint = millis();
  }

  delay(5);
}

/*
 * ═══════════════════════════════════════════════════════════════
 * COSA FA QUESTA VERSIONE:
 * ═══════════════════════════════════════════════════════════════
 *
 * ✅ DEPENDENCY INVERSION (SOLID - D)
 *    Use Cases non conoscono ViessmannRepositoryImpl
 *    Dipendono SOLO da IViessmannRepository (abstract interface)
 *
 * ✅ TESTABILITÀ COMPLETA
 *    Puoi creare MockRepository che implementa IViessmannRepository
 *    Puoi testare Use Cases senza hardware:
 *
 *    class MockRepository : public IViessmannRepository {
 *      bool sendRegister(...) override { /* fake */ }
 *    };
 *
 *    MockRepository mockRepo;
 *    SetTemperatureUseCase uc(model, mockRepo);
 *    uc.execute(22.5);  // Test senza Modbus!
 *
 * ✅ SWAPPABLE IMPLEMENTATION
 *    Domani se cambi da Modbus a WiFi:
 *    - Crei ViessmannRepositoryWiFi : public IViessmannRepository
 *    - Cambi SOLO la instantiazione nel main
 *    - Use Cases NON cambiano!
 *
 * ✅ TRUE SEPARATION OF CONCERNS
 *    Domain (Use Cases) = Business Logic pura
 *    Data (Repository) = Come raggiungere i dati
 *    Model = Stato
 *    ViewModel = Presentazione
 *    View = UI
 *
 * ═══════════════════════════════════════════════════════════════
 */
