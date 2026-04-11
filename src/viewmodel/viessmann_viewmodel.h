#ifndef VIESSMANN_VIEWMODEL_H
#define VIESSMANN_VIEWMODEL_H

#include <string>
#include <cstdio>
#include "../model/viessmann_model_v2.h"

/**
 * ViessmannViewModel - Presentation Layer (MVVM Pattern)
 *
 * Responsabilità:
 * - Ricevere input da View (LVGL callbacks)
 * - Delegare logica al Model
 * - Trasformare dati del Model in formato per View
 * - Notificare View di aggiornare UI
 *
 * È il ponte tra UI (View) e logica (Model).
 * Contiene logica di presentazione (formattazione, validazione UI, ecc)
 * ma NON contiene logica di business.
 */

class ViessmannViewModel {
private:
  ViessmannModel &model;

  // Display strings (trasformati per UI)
  std::string displayTemp;      // "20.5°C"
  std::string displayStatus;    // "FREDDO" o "CALDO"
  std::string displayFanSpeed;  // "MAX", "MIN", ecc
  std::string displayPower;     // "ON" o "OFF"

public:
  // Callback quando View deve aggiornarsi
  std::function<void()> onDisplayUpdate;

  ViessmannViewModel(ViessmannModel &m) : model(m) {
    // Osserva cambiamenti nel Model
    // DISABILITATO: causa deadlock con LVGL
    // model.onStateChanged = [this]() {
    //   onModelStateChanged();
    // };

    // Aggiorna display iniziale
    updateDisplay();
  }

  // ========== INPUT HANDLERS (da View/LVGL) ==========

  void onTemperatureUp() {
    model.increaseTemperature();
  }

  void onTemperatureDown() {
    model.decreaseTemperature();
  }

  void onPowerOn() {
    model.setPower(true);
  }

  void onPowerOff() {
    model.setPower(false);
  }

  void onHeatingToggle() {
    model.setHeating(!model.isHeating());
  }

  void onCoolingToggle() {
    model.setHeating(false);
  }

  void onFanSpeedChanged(int speed) {
    if (speed >= 0 && speed <= 3) {
      model.setFanSpeed((ViessmannModel::FanSpeed)speed);
    }
  }

  void onTemperatureSliderChanged(float value) {
    model.setTemperature(value);
  }

  // ========== OUTPUT GETTERS (per View) ==========

  const char* getDisplayTemp() const {
    return displayTemp.c_str();
  }

  const char* getDisplayStatus() const {
    return displayStatus.c_str();
  }

  const char* getDisplayFanSpeed() const {
    return displayFanSpeed.c_str();
  }

  const char* getDisplayPower() const {
    return displayPower.c_str();
  }

  bool isPowerOn() const {
    return model.isPowerOn();
  }

  bool isHeating() const {
    return model.isHeating();
  }

  float getCurrentTemperature() const {
    return model.getTemperature();
  }

  int getCurrentFanSpeed() const {
    return (int)model.getFanSpeed();
  }

  // ========== PRIVATE HELPERS ==========

private:
  /**
   * Chiamato dal Model quando lo stato cambia
   * Trasforma dati e notifica View
   */
  void onModelStateChanged() {
    updateDisplay();
  }

  /**
   * Trasforma dati del Model in formato per UI
   * Questa è logica di presentazione pura (no business logic)
   */
  void updateDisplay() {
    // Temperatura: "20.5°C"
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f°C", model.getTemperature());
    displayTemp = buf;

    // Stato: "FREDDO" o "CALDO"
    displayStatus = model.isHeating() ? "🔥 CALDO" : "❄️ FREDDO";

    // Velocità ventilatore
    switch (model.getFanSpeed()) {
      case ViessmannModel::FAN_OFF:
        displayFanSpeed = "OFF";
        break;
      case ViessmannModel::FAN_MIN:
        displayFanSpeed = "MIN";
        break;
      case ViessmannModel::FAN_NIGHT:
        displayFanSpeed = "NIGHT";
        break;
      case ViessmannModel::FAN_MAX:
        displayFanSpeed = "MAX";
        break;
      default:
        displayFanSpeed = "?";
    }

    // Potenza
    displayPower = model.isPowerOn() ? "ON" : "OFF";

    // Notifica View che deve aggiornarsi
    if (onDisplayUpdate) {
      onDisplayUpdate();
    }

    Serial.printf("[VIEWMODEL] Aggiornato: %.1f°C %s %s %s\n",
                  model.getTemperature(),
                  displayStatus.c_str(),
                  displayFanSpeed.c_str(),
                  displayPower.c_str());
  }
};

#endif
