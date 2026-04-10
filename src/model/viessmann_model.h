#ifndef VIESSMANN_MODEL_H
#define VIESSMANN_MODEL_H

#include <cstdint>
#include <functional>
#include "modbus_service.h"

/**
 * ViessmannModel - Business Logic Layer (MVVM Pattern)
 *
 * Responsabilità:
 * - Mantenere lo stato del ventilconvettore (temperatura, modalità, power)
 * - Applicare validazioni
 * - Comunicare con ModbusService
 * - Notificare ViewModel di cambiamenti (Observer Pattern)
 *
 * NON sa niente di LVGL o UI.
 * È testabile indipendentemente.
 */

class ViessmannModel {
public:
  // Fan speed enum
  enum FanSpeed {
    FAN_OFF = 0,
    FAN_MIN = 1,
    FAN_NIGHT = 2,
    FAN_MAX = 3
  };

  // Callback quando il modello cambia
  std::function<void()> onStateChanged;

private:
  // Stato interno
  uint16_t regConfig = 0x4003;  // Configuration register (REG 101)
  uint16_t regTemp = 0x00CD;    // Temperature register (REG 102) = 20.5°C
  uint16_t regMode = 0xb9;      // Mode register (REG 103)
  bool powerOn = true;           // Power state
  bool heating = false;          // Heating (true) or Cooling (false)

  ModbusService &modbus;
  const uint8_t MODBUS_BROADCAST = 0x00;
  const uint8_t REG_101 = 101;
  const uint8_t REG_102 = 102;
  const uint8_t REG_103 = 103;

public:
  ViessmannModel(ModbusService &mb) : modbus(mb) {}

  // ========== GETTERS (read-only state) ==========
  bool isPowerOn() const { return powerOn; }
  bool isHeating() const { return heating; }
  float getTemperature() const { return regTemp / 10.0f; }
  FanSpeed getFanSpeed() const { return (FanSpeed)(regConfig & 0x03); }
  uint16_t getRegConfig() const { return regConfig; }
  uint16_t getRegTemp() const { return regTemp; }
  uint16_t getRegMode() const { return regMode; }

  // ========== SETTERS (business logic) ==========

  /**
   * Accendi/spegni il dispositivo
   */
  void setPower(bool on) {
    if (on) {
      // Accensione: FREDDO MAX (default come master originale)
      regConfig = 0x4003;  // bit14 (FREDDO) + FAN MAX
      regMode = 0xb9;
      heating = false;
      powerOn = true;
    } else {
      // Spegnimento: FREDDO + STANDBY
      regConfig = 0x4083;  // bit14 (FREDDO) + bit7 (STANDBY)
      regTemp = 0x32;      // 5.0°C (come master)
      regMode = 0xb9;
      powerOn = false;
    }
    sendAllRegisters();
    notifyChanged();
  }

  /**
   * Imposta modalità riscaldamento/raffrescamento
   */
  void setHeating(bool h) {
    heating = h;
    if (powerOn) {
      if (h) {
        // CALDO: bit13=1, bit14=0
        regConfig &= ~(1 << 14);  // Spegni FREDDO
        regConfig |= (1 << 13);   // Accendi CALDO
        regMode |= 0x02;
      } else {
        // FREDDO: bit14=1, bit13=0
        regConfig &= ~(1 << 13);  // Spegni CALDO
        regConfig |= (1 << 14);   // Accendi FREDDO
        regMode &= ~0x02;
      }
    }
    sendAllRegisters();
    notifyChanged();
  }

  /**
   * Imposta velocità ventilatore (0-3)
   */
  void setFanSpeed(FanSpeed speed) {
    regConfig = (regConfig & ~0x03) | (speed & 0x03);
    sendAllRegisters();
    notifyChanged();
  }

  /**
   * Imposta temperatura setpoint (5.0 - 24.0°C)
   * Validazione automatica
   */
  void setTemperature(float temp) {
    // Validazione
    if (temp < 5.0f) temp = 5.0f;
    if (temp > 24.0f) temp = 24.0f;

    regTemp = (uint16_t)(temp * 10);
    sendAllRegisters();
    notifyChanged();
  }

  /**
   * Aumenta temperatura di 0.5°C
   */
  void increaseTemperature() {
    setTemperature(getTemperature() + 0.5f);
  }

  /**
   * Diminuisci temperatura di 0.5°C
   */
  void decreaseTemperature() {
    setTemperature(getTemperature() - 0.5f);
  }

  // ========== PRIVATE HELPERS ==========

private:
  /**
   * Invia tutti e tre i registri al dispositivo (come il master originale)
   * REG 101 (config), REG 102 (temp), REG 103 (mode)
   */
  void sendAllRegisters() {
#ifdef SNIFFER_MODE
    Serial.println("[MODEL] SNIFFER_MODE: invio disabilitato");
    return;
#endif
    Serial.println("[MODEL] Invio registri...");
    modbus.writeRegister(MODBUS_BROADCAST, modbus.regAddress(REG_101), regConfig);
    delay(500);
    modbus.writeRegister(MODBUS_BROADCAST, modbus.regAddress(REG_102), regTemp);
    delay(500);
    modbus.writeRegister(MODBUS_BROADCAST, modbus.regAddress(REG_103), regMode);

    Serial.printf("  REG101=0x%04X REG102=0x%04X(%.1f°C) REG103=0x%04X OK\n",
                  regConfig, regTemp, getTemperature(), regMode);
  }

  /**
   * Notifica ViewModel che lo stato è cambiato
   * (Observer Pattern)
   */
  void notifyChanged() {
    if (onStateChanged) {
      onStateChanged();
    }
  }
};

#endif
