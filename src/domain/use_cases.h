#ifndef USE_CASES_H
#define USE_CASES_H

#include "../data/viessmann_repository.h"
#include <functional>

/**
 * Use Cases - Domain/Business Logic Layer
 *
 * Responsabilità:
 * - Implementare casi d'uso dell'applicazione
 * - Contengono business logic (validazione, calcoli, ecc)
 * - Ricevono Repository tramite dependency injection
 * - Sono testabili indipendentemente
 *
 * Clean Architecture: Layer "Use Cases" (Business Rules)
 * Rappresentano le azioni che l'utente può fare.
 */

// Forward declaration
class ViessmannModel;

// ========== USE CASE: SET TEMPERATURE ==========

class SetTemperatureUseCase {
private:
  ViessmannModel &model;
  ViessmannRepository &repository;

public:
  SetTemperatureUseCase(ViessmannModel &m, ViessmannRepository &r)
      : model(m), repository(r) {}

  /**
   * Imposta la temperatura con validazione
   */
  void execute(float temperature) {
    // Validazione (business logic)
    if (temperature < 5.0f) temperature = 5.0f;
    if (temperature > 24.0f) temperature = 24.0f;

    // Calcola registro
    uint16_t regValue = (uint16_t)(temperature * 10);

    // Persisti
    repository.sendRegister(102, regValue);

    Serial.printf("[UC] SetTemperature: %.1f°C (0x%04X)\n", temperature, regValue);
  }
};

// ========== USE CASE: TOGGLE POWER ==========

class TogglePowerUseCase {
private:
  ViessmannModel &model;
  ViessmannRepository &repository;

public:
  TogglePowerUseCase(ViessmannModel &m, ViessmannRepository &r)
      : model(m), repository(r) {}

  void executePowerOn() {
    // Business logic per accensione
    uint16_t regConfig = 0x4003;  // FREDDO MAX
    uint16_t regTemp = 0x00CD;    // 20.5°C
    uint16_t regMode = 0xb9;

    repository.sendAllRegisters(regConfig, regTemp, regMode);
    Serial.println("[UC] TogglePower: ON");
  }

  void executePowerOff() {
    // Business logic per spegnimento
    uint16_t regConfig = 0x4083;  // FREDDO + STANDBY
    uint16_t regTemp = 0x32;      // 5.0°C
    uint16_t regMode = 0xb9;

    repository.sendAllRegisters(regConfig, regTemp, regMode);
    Serial.println("[UC] TogglePower: OFF");
  }
};

// ========== USE CASE: CHANGE MODE (HEAT/COOL) ==========

class ChangeHeatingModeUseCase {
private:
  ViessmannModel &model;
  ViessmannRepository &repository;

public:
  ChangeHeatingModeUseCase(ViessmannModel &m, ViessmannRepository &r)
      : model(m), repository(r) {}

  void executeHeating(uint16_t currentRegConfig) {
    // Calcola nuovo valore: bit14=0 (FREDDO OFF), bit13=1 (CALDO ON)
    uint16_t regConfig = currentRegConfig;
    regConfig &= ~(1 << 14);  // Spegni FREDDO
    regConfig |= (1 << 13);   // Accendi CALDO

    repository.sendRegister(101, regConfig);
    Serial.println("[UC] ChangeHeatingMode: HEAT");
  }

  void executeCooling(uint16_t currentRegConfig) {
    // Calcola nuovo valore: bit14=1 (FREDDO ON), bit13=0 (CALDO OFF)
    uint16_t regConfig = currentRegConfig;
    regConfig &= ~(1 << 13);  // Spegni CALDO
    regConfig |= (1 << 14);   // Accendi FREDDO

    repository.sendRegister(101, regConfig);
    Serial.println("[UC] ChangeHeatingMode: COOL");
  }
};

// ========== USE CASE: CHANGE FAN SPEED ==========

class ChangeFanSpeedUseCase {
private:
  ViessmannModel &model;
  ViessmannRepository &repository;

public:
  ChangeFanSpeedUseCase(ViessmannModel &m, ViessmannRepository &r)
      : model(m), repository(r) {}

  void execute(int speed, uint16_t currentRegConfig) {
    if (speed < 0 || speed > 3) {
      Serial.println("[UC] Invalid fan speed");
      return;
    }

    // Modifica bit 0-1 del registro
    uint16_t regConfig = currentRegConfig;
    regConfig = (regConfig & ~0x03) | (speed & 0x03);

    repository.sendRegister(101, regConfig);
    Serial.printf("[UC] ChangeFanSpeed: %d\n", speed);
  }
};

#endif
