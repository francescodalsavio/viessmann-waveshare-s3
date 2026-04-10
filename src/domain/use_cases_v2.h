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
    if (temperature < 5.0f) temperature = 5.0f;      // Min: 0x32 (0000 0000 0011 0010)
    if (temperature > 24.0f) temperature = 24.0f;    // Max: 0xF0 (0000 0000 1111 0000)

    // Calcola registro (formula: Celsius × 10 = hex value)
    // Es: 22.5°C × 10 = 225 = 0xE1 (1110 0001)
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
    uint16_t regConfig = 0x4003;  // 0100 0000 0000 0011 — FREDDO MAX + ON
    uint16_t regTemp = 0x00CD;    // 0000 0000 1100 1101 — 20.5°C
    uint16_t regMode = 0xb9;      // 1011 1001 — Seasonal mode

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
    uint16_t regConfig = 0x4083;  // 0100 0000 1000 0011 — FREDDO + STANDBY
    uint16_t regTemp = 0x32;      // 0000 0000 0011 0010 — 5.0°C (reset)
    uint16_t regMode = 0xb9;      // 1011 1001 — Seasonal mode

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
    // Flip bit14(FREDDO)=0, bit13(CALDO)=1
    // Risultato: 0x2003 (0010 0000 0000 0011)
    regConfig &= ~(1 << 14);  // Spegni bit 14 (FREDDO)
    regConfig |= (1 << 13);   // Accendi bit 13 (CALDO)

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
    // Flip bit13(CALDO)=0, bit14(FREDDO)=1
    // Risultato: 0x4003 (0100 0000 0000 0011)
    regConfig &= ~(1 << 13);  // Spegni bit 13 (CALDO)
    regConfig |= (1 << 14);   // Accendi bit 14 (FREDDO)

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
    // Speed: 0=OFF(00), 1=MIN(01), 2=AUTO(10), 3=MAX(11)
    if (speed < 0 || speed > 3) {
      if (logger) {
        logger->error("[UC] Invalid fan speed");
      }
      return;
    }

    uint16_t regConfig = currentRegConfig;
    // Modifica bit1-0 (FAN speed), mantieni resto
    // Es: speed=3 → regConfig |= 0b11 → 0x4003 (0100 0000 0000 0011)
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
