#ifndef VIESSMANN_REPOSITORY_IMPL_H
#define VIESSMANN_REPOSITORY_IMPL_H

#include "../domain/i_viessmann_repository.h"
#include "../model/modbus_service.h"
#include <cstdint>

/**
 * ViessmannRepositoryImpl - Concrete Implementation (Data Layer)
 *
 * Implementa IViessmannRepository usando ModbusService.
 * Questa è l'unica classe che "sa" come parlare a Modbus.
 *
 * Il Domain (Use Cases) NON conosce questa classe.
 * Dipende solo dall'interface IViessmannRepository.
 */

class ViessmannRepositoryImpl : public IViessmannRepository {
private:
  ModbusService &modbus;
  const uint8_t MODBUS_BROADCAST = 0x00;

public:
  ViessmannRepositoryImpl(ModbusService &mb) : modbus(mb) {}

  /**
   * Implementazione: invia un singolo registro via Modbus
   */
  bool sendRegister(uint8_t regNumber, uint16_t value) override {
    try {
      modbus.writeRegister(MODBUS_BROADCAST, modbus.regAddress(regNumber), value);
      Serial.printf("[REPO-IMPL] sendRegister(REG%d, 0x%04X) OK\n", regNumber, value);
      return true;
    } catch (...) {
      Serial.printf("[REPO-IMPL] sendRegister(REG%d) FAILED\n", regNumber);
      return false;
    }
  }

  /**
   * Implementazione: invia tutti e tre i registri
   */
  bool sendAllRegisters(uint16_t regConfig, uint16_t regTemp, uint16_t regMode) override {
    try {
      Serial.println("[REPO-IMPL] Sending all registers...");

      if (!sendRegister(101, regConfig)) return false;
      delay(500);

      if (!sendRegister(102, regTemp)) return false;
      delay(500);

      if (!sendRegister(103, regMode)) return false;

      Serial.printf("[REPO-IMPL] All registers sent: 0x%04X 0x%04X 0x%04X\n",
                    regConfig, regTemp, regMode);
      return true;
    } catch (...) {
      Serial.println("[REPO-IMPL] sendAllRegisters FAILED");
      return false;
    }
  }

  /**
   * Implementazione: legge un registro (placeholder per future espansioni)
   */
  uint16_t readRegister(uint8_t regNumber) override {
    // Implementazione futura per leggere stato dal dispositivo
    Serial.printf("[REPO-IMPL] readRegister(REG%d) - NOT IMPLEMENTED\n", regNumber);
    return 0;
  }
};

#endif
