/*
 * ═════════════════════════════════════════════════════════════
 * VISLA — CLEAN ARCHITECTURE v3.0
 * ═════════════════════════════════════════════════════════════
 *
 * Architettura completa con:
 * - Use Cases (logica applicazione)
 * - Repository (data access)
 * - Model (stato)
 * - ViewModel (presentazione)
 * - View (UI)
 *
 * Livelli (da interno a esterno):
 *
 * ┌──────────────────────────────────────────┐
 * │  View Layer (LVGL UI)                    │
 * │  - Render                                │
 * │  - Event handling                        │
 * └──────────────────┬───────────────────────┘
 *                    │
 * ┌──────────────────▼───────────────────────┐
 * │  ViewModel Layer (Presentation)          │
 * │  - Data transformation                   │
 * │  - State management                      │
 * └──────────────────┬───────────────────────┘
 *                    │
 * ┌──────────────────▼───────────────────────┐
 * │  Domain Layer (Use Cases)                │
 * │  - SetTemperatureUseCase                 │
 * │  - TogglePowerUseCase                    │
 * │  - ChangeHeatingModeUseCase              │
 * │  - ChangeFanSpeedUseCase                 │
 * └──────────────────┬───────────────────────┘
 *                    │
 * ┌──────────────────▼───────────────────────┐
 * │  Model Layer (Entity State)              │
 * │  - regConfig, regTemp, regMode           │
 * │  - powerOn, heating                      │
 * └──────────────────┬───────────────────────┘
 *                    │
 * ┌──────────────────▼───────────────────────┐
 * │  Data Layer (Repository)                 │
 * │  - ViessmannRepository (Modbus abstraction)
 * │  - sendRegister(), sendAllRegisters()    │
 * └──────────────────┬───────────────────────┘
 *                    │
 * ┌──────────────────▼───────────────────────┐
 * │  Devices/Hardware Layer                  │
 * │  - ModbusService (RS485)                 │
 * │  - WebServer (HTTP)                      │
 * │  - LVGL (Display)                        │
 * └──────────────────────────────────────────┘
 *
 * FLUSSO DI UN COMANDO:
 * UI Button Click
 *   ↓
 * View.onTemperatureUp() → ViewModel
 *   ↓
 * ViewModel.onTemperatureUp() → Model
 *   ↓
 * Model.increaseTemperature() → UseCase
 *   ↓
 * SetTemperatureUseCase.execute() → Repository
 *   ↓
 * Repository.sendRegister() → ModbusService
 *   ↓
 * ModbusService.writeRegister() → RS485 Hardware
 *
 * ═════════════════════════════════════════════════════════════
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
#include "data/viessmann_repository.h"

// ========== LAYER 2: DOMAIN (USE CASES) ==========
#include "domain/use_cases.h"

// ========== LAYER 3: MODEL (ENTITY STATE) ==========
#include "model/viessmann_model_v2.h"

// ========== LAYER 4: PRESENTATION ==========
#include "viewmodel/viessmann_viewmodel.h"
#include "view/viessmann_view.h"

// ========== GLOBAL INSTANCES (Dependency Injection) ==========

// Layer 0
ModbusService modbus;

// Layer 1
ViessmannRepository repository(modbus);

// Layer 3
ViessmannModel model;

// Layer 2 (Use Cases - inject Model e Repository)
SetTemperatureUseCase setTempUC(model, repository);
TogglePowerUseCase togglePowerUC(model, repository);
ChangeHeatingModeUseCase changeModeUC(model, repository);
ChangeFanSpeedUseCase changeFanUC(model, repository);

// Layer 4
ViessmannViewModel viewModel(model);
ViessmannView view(viewModel);

// Web Server
WebServer webServer(80);

// ========== CONFIGURATION ==========
const char* WIFI_SSID = "Molinella";
const char* WIFI_PASS = "Fastweb10";

// ========== WEB API HANDLERS ==========

void handleStatus() {
  String json = "{";
  json += "\"power\":\"" + String(model.isPowerOn() ? "on" : "off") + "\",";
  json += "\"temperature\":" + String(model.getTemperature(), 1) + ",";
  json += "\"mode\":\"" + String(model.isHeating() ? "heat" : "cool") + "\",";
  json += "\"fan_speed\":" + String((int)model.getFanSpeed()) + ",";
  json += "\"architecture\":\"Clean Architecture v3.0 (Use Cases + Repository)\"";
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
  Serial.println("[BOOT] Web Server started on port 80");
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

  Serial.println("\n╔═══════════════════════════════════════════╗");
  Serial.println("║  VISLA — Clean Architecture v3.0          ║");
  Serial.println("║  Use Cases + Repository + MVVM             ║");
  Serial.println("╚═══════════════════════════════════════════╝\n");

  // Inject Use Cases nel Model
  Serial.println("[BOOT] Injecting Use Cases...");
  model.injectUseCases(&setTempUC, &togglePowerUC, &changeModeUC, &changeFanUC);

  // Inizializza ModbusService
  Serial.println("[BOOT] Initializing Modbus...");
  modbus.begin();

  // Inizializza WiFi
  Serial.println("[BOOT] Initializing WiFi...");
  setupWiFi();

  // Inizializza Web Server
  Serial.println("[BOOT] Initializing Web Server...");
  setupWebServer();

  // Inizializza LVGL
  Serial.println("[BOOT] Initializing LVGL...");
  lv_init();
  lv_tick_set_cb(millis_cb);

  // Crea UI
  Serial.println("[BOOT] Creating UI (View)...");
  view.create();

  // Invia stato iniziale
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
    Serial.printf("[LOOP] State: Temp=%.1f°C Power=%s Mode=%s Fan=%d\n",
                  model.getTemperature(),
                  model.isPowerOn() ? "ON" : "OFF",
                  model.isHeating() ? "HEAT" : "COOL",
                  (int)model.getFanSpeed());
    lastLoopPrint = millis();
  }

  delay(5);
}

/*
 * ═════════════════════════════════════════════════════════════
 * COSA ABBIAMO REALIZZATO:
 * ═════════════════════════════════════════════════════════════
 *
 * ✅ MVVM Pattern (Model-View-ViewModel)
 *    - Model: stato del ventilconvettore
 *    - ViewModel: trasforma dati per UI, orchestra input
 *    - View: LVGL UI, niente logica
 *
 * ✅ Use Cases (Domain Logic)
 *    - SetTemperatureUseCase
 *    - TogglePowerUseCase
 *    - ChangeHeatingModeUseCase
 *    - ChangeFanSpeedUseCase
 *    Contengono la logica di business isolata e testabile
 *
 * ✅ Repository Pattern (Data Access)
 *    - ViessmannRepository astrae Modbus
 *    - sendRegister(), sendAllRegisters()
 *    - Facilmente sostituibile
 *
 * ✅ Dependency Injection
 *    - Use Cases ricevono Model e Repository
 *    - Model riceve Use Cases tramite injectUseCases()
 *    - Niente global tight coupling
 *
 * ✅ Clean Architecture
 *    - Devices/Hardware (ModbusService, WebServer, LVGL)
 *    - Data Access (Repository)
 *    - Domain/Business (Use Cases)
 *    - Application (Model, ViewModel)
 *    - Presentation (View)
 *
 * VANTAGGI:
 *
 * 🎯 Testabilità
 *    - Puoi testare Use Cases senza Model
 *    - Puoi testare Model senza View
 *    - Puoi mockare Repository per testing
 *
 * 🎯 Manutenibilità
 *    - Ogni layer ha UNA responsabilità
 *    - Cambi UI? Solo View cambia
 *    - Cambi hardware? Solo Repository cambia
 *
 * 🎯 Scalabilità
 *    - Aggiungi feature? Aggiungi un nuovo UseCase
 *    - Non tocchi il resto del codice
 *
 * 🎯 Indipendenza dai Framework
 *    - Model e Use Cases NON conoscono LVGL
 *    - Repository NON conosce i dettagli Modbus
 *    - Potresti usare stessa logica su web, mobile, etc
 *
 * 🎯 Reusabilità
 *    - Use Cases possono essere riusati in altri UI
 *    - Model può essere usato in un'app console
 *    - Repository può essere swappato facilmente
 *
 * ═════════════════════════════════════════════════════════════
 */
