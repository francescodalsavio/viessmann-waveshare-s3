#ifndef VIESSMANN_MODEL_V2_H
#define VIESSMANN_MODEL_V2_H

#include <cstdint>
#include <functional>
#include "../domain/use_cases_v2.h"

/**
 * ViessmannModel v2 - Con Use Cases
 *
 * Refactorizzato per usare Use Cases.
 * Mantiene lo stato ma delega la logica ai Use Cases.
 *
 * Clean Architecture:
 * Model è il custode dello stato
 * Use Cases contengono la logica
 */

class ViessmannModel {
public:
  enum FanSpeed {
    FAN_OFF = 0,
    FAN_MIN = 1,
    FAN_NIGHT = 2,
    FAN_MAX = 3
  };

  std::function<void()> onStateChanged;

private:
  uint16_t regConfig = 0x4003;
  uint16_t regTemp = 0x00E6;  // 23.0°C ← Come fa il Master originale!
  uint16_t regMode = 0xb9;
  bool powerOn = true;
  bool heating = false;

  // Use Cases (dependency injection)
  SetTemperatureUseCase *setTempUC = nullptr;
  TogglePowerUseCase *togglePowerUC = nullptr;
  ChangeHeatingModeUseCase *changeModeUC = nullptr;
  ChangeFanSpeedUseCase *changeFanUC = nullptr;

public:
  ViessmannModel() {}

  // Dependency Injection per Use Cases
  void injectUseCases(
      SetTemperatureUseCase *setTemp,
      TogglePowerUseCase *togglePower,
      ChangeHeatingModeUseCase *changeMode,
      ChangeFanSpeedUseCase *changeFan) {
    setTempUC = setTemp;
    togglePowerUC = togglePower;
    changeModeUC = changeMode;
    changeFanUC = changeFan;
  }

  // ========== GETTERS ==========
  bool isPowerOn() const { return powerOn; }
  bool isHeating() const { return heating; }
  float getTemperature() const { return regTemp / 10.0f; }
  FanSpeed getFanSpeed() const { return (FanSpeed)(regConfig & 0x03); }
  uint16_t getRegConfig() const { return regConfig; }
  uint16_t getRegTemp() const { return regTemp; }
  uint16_t getRegMode() const { return regMode; }

  // ========== SETTERS (delegano ai Use Cases) ==========

  void setPower(bool on) {
    if (!togglePowerUC) return;

    if (on) {
      togglePowerUC->executePowerOn();
      regConfig = 0x4003;
      regMode = 0xb9;
      heating = false;
      powerOn = true;
    } else {
      togglePowerUC->executePowerOff();
      regConfig = 0x4083;
      regTemp = 0x32;
      regMode = 0xb9;
      powerOn = false;
    }

    notifyChanged();
  }

  void setTemperature(float temp) {
    if (!setTempUC) return;

    setTempUC->execute(temp);
    regTemp = (uint16_t)(temp * 10);

    notifyChanged();
  }

  void increaseTemperature() {
    setTemperature(getTemperature() + 0.5f);
  }

  void decreaseTemperature() {
    setTemperature(getTemperature() - 0.5f);
  }

  void setHeating(bool h) {
    if (!changeModeUC) return;

    heating = h;

    if (powerOn) {
      if (h) {
        changeModeUC->executeHeating(regConfig);
        regConfig &= ~(1 << 14);
        regConfig |= (1 << 13);
        regMode |= 0x02;
      } else {
        changeModeUC->executeCooling(regConfig);
        regConfig &= ~(1 << 13);
        regConfig |= (1 << 14);
        regMode &= ~0x02;
      }
    }

    notifyChanged();
  }

  void setFanSpeed(FanSpeed speed) {
    if (!changeFanUC) return;

    changeFanUC->execute((int)speed, regConfig);
    regConfig = (regConfig & ~0x03) | (speed & 0x03);

    notifyChanged();
  }

private:
  void notifyChanged() {
    if (onStateChanged) {
      onStateChanged();
    }
  }
};

#endif
