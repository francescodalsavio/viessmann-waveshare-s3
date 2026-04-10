/*
 * VISLA Modbus Display — MVVM + Clean Architecture
 *
 * Architettura MVVM:
 * ┌─────────────────────────────────────────┐
 * │  View (LVGL UI)                         │
 * │  - Render widgets                       │
 * │  - Forward events a ViewModel           │
 * └────────────────┬────────────────────────┘
 *                  │
 * ┌────────────────▼────────────────────────┐
 * │  ViewModel (Presentation Logic)         │
 * │  - Trasforma dati per UI                │
 * │  - Orchestrazione tra View e Model      │
 * └────────────────┬────────────────────────┘
 *                  │
 * ┌────────────────▼────────────────────────┐
 * │  Model (Business Logic)                 │
 * │  - Stato ventilconvettore               │
 * │  - Logica temperatura/modalità          │
 * │  - Comunicazione Modbus                 │
 * └─────────────────────────────────────────┘
 */

#define SNIFFER_MODE  // Decommentare per ascolto passivo

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <lvgl.h>
#include "lvgl_v8_port.h"

// Import architettura MVVM
#include "model/modbus_service.h"
#include "model/viessmann_model.h"
#include "viewmodel/viessmann_viewmodel.h"
#include "view/viessmann_view.h"

// === WiFi Config ===
const char* WIFI_SSID = "Molinella";
const char* WIFI_PASS = "Fastweb10";

// === Global Instances ===
ModbusService modbus;
ViessmannModel model(modbus);
ViessmannViewModel viewModel(model);
ViessmannView view(viewModel);

WebServer webServer(80);

// === Web API Handlers ===

void handleStatus() {
  String json = "{";
  json += "\"power\":\"" + String(model.isPowerOn() ? "on" : "off") + "\",";
  json += "\"temperature\":" + String(model.getTemperature(), 1) + ",";
  json += "\"mode\":\"" + String(model.isHeating() ? "heat" : "cool") + "\",";
  json += "\"fan_speed\":" + String((int)model.getFanSpeed()) + ",";
  json += "\"reg101\":\"0x" + String(model.getRegConfig(), HEX) + "\",";
  json += "\"reg102\":\"0x" + String(model.getRegTemp(), HEX) + "\",";
  json += "\"reg103\":\"0x" + String(model.getRegMode(), HEX) + "\"";
  json += "}";

  webServer.send(200, "application/json", json);
  Serial.printf("[API] GET /status: %s\n", json.c_str());
}

void handlePowerOn() {
  model.setPower(true);
  webServer.send(200, "text/plain", "Power ON");
  Serial.println("[API] POST /power/on");
}

void handlePowerOff() {
  model.setPower(false);
  webServer.send(200, "text/plain", "Power OFF");
  Serial.println("[API] POST /power/off");
}

void handleSetTemperature() {
  if (webServer.hasArg("value")) {
    float temp = webServer.arg("value").toFloat();
    model.setTemperature(temp);
    webServer.send(200, "text/plain", "Temperature set");
    Serial.printf("[API] POST /temperature?value=%.1f\n", temp);
  } else {
    webServer.send(400, "text/plain", "Missing 'value' parameter");
  }
}

void handleSetMode() {
  if (webServer.hasArg("mode")) {
    String mode = webServer.arg("mode");
    bool heating = (mode == "heat");
    model.setHeating(heating);
    webServer.send(200, "text/plain", heating ? "Heating ON" : "Cooling ON");
    Serial.printf("[API] POST /mode?mode=%s\n", mode.c_str());
  } else {
    webServer.send(400, "text/plain", "Missing 'mode' parameter");
  }
}

void handleSetFanSpeed() {
  if (webServer.hasArg("speed")) {
    int speed = webServer.arg("speed").toInt();
    model.setFanSpeed((ViessmannModel::FanSpeed)speed);
    webServer.send(200, "text/plain", "Fan speed set");
    Serial.printf("[API] POST /fan?speed=%d\n", speed);
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
  Serial.println("[WEB] Server started on port 80");
}

void setupWiFi() {
  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("[WiFi] Connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("[WiFi] Failed to connect");
  }
}

// === LVGL Tick ===

static uint32_t millis_cb(void) {
  return millis();
}

// === Arduino Setup ===

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  VISLA Modbus Display — MVVM v2.0      ║");
  Serial.println("║  Viessmann Energycal Slim Controller    ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // Inizializza Modbus
  Serial.println("[BOOT] Initializing Modbus...");
  modbus.begin();

  // Configura WiFi
  Serial.println("[BOOT] Initializing WiFi...");
  setupWiFi();

  // Configura Web Server
  Serial.println("[BOOT] Initializing Web Server...");
  setupWebServer();

  // Inizializza LVGL
  Serial.println("[BOOT] Initializing LVGL...");
  lv_init();
  lv_tick_set_cb(millis_cb);

  // Initialize display hardware (dipende dal tuo setup)
  // ... (configurazione display/touchscreen)

  // Crea UI
  Serial.println("[BOOT] Creating UI...");
  view.create();

  // Invia registri iniziali
  Serial.println("[BOOT] Sending initial registers...");
  model.setPower(true);

  Serial.println("[BOOT] Setup complete!\n");
}

// === Arduino Loop ===

uint32_t lastLoopPrint = 0;

void loop() {
  // Gestisci Web Server
  webServer.handleClient();

  // Aggiorna LVGL
  lv_timer_handler();

  // Log periodico (ogni 10 secondi)
  if (millis() - lastLoopPrint > 10000) {
    Serial.printf("[LOOP] Temp=%.1f°C Power=%s Mode=%s FanSpeed=%d\n",
                  model.getTemperature(),
                  model.isPowerOn() ? "ON" : "OFF",
                  model.isHeating() ? "HEAT" : "COOL",
                  (int)model.getFanSpeed());
    lastLoopPrint = millis();
  }

  delay(5);
}

/*
 * ════════════════════════════════════════════════════════════════
 * ARCHITETTURA SPIEGAZIONE:
 * ════════════════════════════════════════════════════════════════
 *
 * 1. MODEL (viessmann_model.h)
 *    - Mantiene stato: regConfig, regTemp, regMode, powerOn, heating
 *    - Fornisce getters/setters: getTemperature(), setTemperature(), etc
 *    - Comunica con Modbus via ModbusService
 *    - Notifica ViewModel quando stato cambia (onStateChanged callback)
 *    - ZERO dipendenze da LVGL
 *
 * 2. VIEWMODEL (viessmann_viewmodel.h)
 *    - Riceve input da View: onTemperatureUp(), onPowerOn(), etc
 *    - Chiama metodi Model: model.setTemperature(), model.setPower(), etc
 *    - Trasforma dati per UI: getDisplayTemp() ritorna "20.5°C"
 *    - Notifica View quando necessario aggiornare: onDisplayUpdate callback
 *    - È il "collante" tra View e Model
 *
 * 3. VIEW (viessmann_view.h)
 *    - Crea widget LVGL: label_temp, btn_power_on, etc
 *    - Event callbacks forwarda a ViewModel: viewModel.onTemperatureUp()
 *    - refreshDisplay() legge valori da ViewModel e aggiorna UI
 *    - ZERO logica di business
 *
 * FLOW ESEMPIO - Utente clicca "Temp +"
 *    1. LVGL genera evento click su btn_temp_up
 *    2. btn_temp_up_callback() chiama: view.viewModel.onTemperatureUp()
 *    3. ViewModel chiama: model.increaseTemperature()
 *    4. Model:
 *       - Calcola: newTemp = currentTemp + 0.5
 *       - Valida: if (newTemp < 5.0) newTemp = 5.0
 *       - Invia Modbus: modbus.writeRegister(102, newReg)
 *       - Chiama: onStateChanged() callback
 *    5. Model notify ViewModel tramite onStateChanged()
 *    6. ViewModel riceve notifica, chiama: updateDisplay()
 *    7. ViewModel trasforma: displayTemp = "20.5°C"
 *    8. ViewModel notifica View: onDisplayUpdate()
 *    9. View refreshDisplay() aggiorna label_temp con nuovo valore
 *
 * VANTAGGI:
 * ✅ Testabile: puoi testare Model senza LVGL
 * ✅ Mantenibile: separa UI, logica, dati
 * ✅ Scalabile: aggiungi feature senza incasinare il codice
 * ✅ Riusabile: stessi Model/ViewModel per altri UI framework
 */
