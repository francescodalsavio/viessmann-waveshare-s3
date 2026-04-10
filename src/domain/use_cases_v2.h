#ifndef USE_CASES_V2_H
#define USE_CASES_V2_H

#include "./i_viessmann_repository.h"
#include "../infrastructure/i_logger.h"
#include <functional>
#include <cstdio>

/**
 * Use Cases v2 - Con Dependency Inversion
 *
 * I Use Cases dipendono SOLO dall'interface IViessmannRepository.
 * NON conoscono l'implementazione concreta (ModbusService, ecc).
 *
 * SOLID - Dependency Inversion Principle:
 * "Depend upon abstractions, not upon concretions"
 *
 * Opzionalmente dipendono da ILogger (interface) per logging.
 * Se logger è nullptr, no logging (per tests).
 *
 * Questo consente:
 * ✅ Testing con MockRepository
 * ✅ Swappare implementazione
 * ✅ Vera separazione Domain ↔ Data
 * ✅ Framework-agnostic logging
 */

// Forward declaration
class ViessmannModel;

// ========== USE CASE: SET TEMPERATURE ==========

class SetTemperatureUseCase {
private:
  ViessmannModel &model;
  IViessmannRepository &repository;  // ← Dipende dall'interface, NON dall'impl
  ILogger *logger;                     // ← Optional logger (nullptr = no logging)

public:
  SetTemperatureUseCase(ViessmannModel &m, IViessmannRepository &r, ILogger *l = nullptr)
      : model(m), repository(r), logger(l) {}

  void execute(float temperature) {
    // Validazione (business logic pura)
    if (temperature < 5.0f) temperature = 5.0f;
    if (temperature > 24.0f) temperature = 24.0f;

    // Calcola registro
    uint16_t regValue = (uint16_t)(temperature * 10);

    // Persisti tramite interface
    Result<void> result = repository.sendRegister(102, regValue);

    // Log se logger disponibile
    if (logger) {
      char buf[64];
      if (result.isOk()) {
        snprintf(buf, sizeof(buf), "[UC] SetTemperature: %.1f°C ✓", temperature);
      } else {
        snprintf(buf, sizeof(buf), "[UC] SetTemperature: %.1f°C ✗ (%s)",
                 temperature, result.errorMessage());
      }
      logger->info(buf);
    }
  }
};

// ========== USE CASE: TOGGLE POWER ==========

class TogglePowerUseCase {
private:
  ViessmannModel &model;
  IViessmannRepository &repository;  // ← Interface
  ILogger *logger;                    // ← Optional logger

public:
  TogglePowerUseCase(ViessmannModel &m, IViessmannRepository &r, ILogger *l = nullptr)
      : model(m), repository(r), logger(l) {}

  void executePowerOn() {
    uint16_t regConfig = 0x4003;  // FREDDO MAX
    uint16_t regTemp = 0x00CD;    // 20.5°C
    uint16_t regMode = 0xb9;

    Result<void> result = repository.sendAllRegisters(regConfig, regTemp, regMode);
    if (logger) {
      if (result.isOk()) {
        logger->info("[UC] TogglePower: ON ✓");
      } else {
        logger->error(result.errorMessage());
      }
    }
  }

  void executePowerOff() {
    uint16_t regConfig = 0x4083;  // FREDDO + STANDBY
    uint16_t regTemp = 0x32;      // 5.0°C
    uint16_t regMode = 0xb9;

    Result<void> result = repository.sendAllRegisters(regConfig, regTemp, regMode);
    if (logger) {
      if (result.isOk()) {
        logger->info("[UC] TogglePower: OFF ✓");
      } else {
        logger->error(result.errorMessage());
      }
    }
  }
};

// ========== USE CASE: CHANGE MODE (HEAT/COOL) ==========

class ChangeHeatingModeUseCase {
private:
  ViessmannModel &model;
  IViessmannRepository &repository;  // ← Interface
  ILogger *logger;                    // ← Optional logger

public:
  ChangeHeatingModeUseCase(ViessmannModel &m, IViessmannRepository &r, ILogger *l = nullptr)
      : model(m), repository(r), logger(l) {}

  void executeHeating(uint16_t currentRegConfig) {
    uint16_t regConfig = currentRegConfig;
    regConfig &= ~(1 << 14);  // Spegni FREDDO
    regConfig |= (1 << 13);   // Accendi CALDO

    Result<void> result = repository.sendRegister(101, regConfig);
    if (logger) {
      if (result.isOk()) {
        logger->info("[UC] ChangeHeatingMode: HEAT ✓");
      } else {
        logger->error(result.errorMessage());
      }
    }
  }

  void executeCooling(uint16_t currentRegConfig) {
    uint16_t regConfig = currentRegConfig;
    regConfig &= ~(1 << 13);  // Spegni CALDO
    regConfig |= (1 << 14);   // Accendi FREDDO

    Result<void> result = repository.sendRegister(101, regConfig);
    if (logger) {
      if (result.isOk()) {
        logger->info("[UC] ChangeHeatingMode: COOL ✓");
      } else {
        logger->error(result.errorMessage());
      }
    }
  }
};

// ========== USE CASE: CHANGE FAN SPEED ==========

class ChangeFanSpeedUseCase {
private:
  ViessmannModel &model;
  IViessmannRepository &repository;  // ← Interface
  ILogger *logger;                    // ← Optional logger

public:
  ChangeFanSpeedUseCase(ViessmannModel &m, IViessmannRepository &r, ILogger *l = nullptr)
      : model(m), repository(r), logger(l) {}

  void execute(int speed, uint16_t currentRegConfig) {
    if (speed < 0 || speed > 3) {
      if (logger) {
        logger->error("[UC] Invalid fan speed");
      }
      return;
    }

    uint16_t regConfig = currentRegConfig;
    regConfig = (regConfig & ~0x03) | (speed & 0x03);

    Result<void> result = repository.sendRegister(101, regConfig);
    if (logger) {
      char buf[48];
      if (result.isOk()) {
        snprintf(buf, sizeof(buf), "[UC] ChangeFanSpeed: %d ✓", speed);
      } else {
        snprintf(buf, sizeof(buf), "[UC] ChangeFanSpeed: %d ✗ (%s)", speed, result.errorMessage());
      }
      logger->info(buf);
    }
  }
};

#endif
