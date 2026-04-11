#ifndef VIESSMANN_REPOSITORY_IMPL_H
#define VIESSMANN_REPOSITORY_IMPL_H

#include "../domain/i_viessmann_repository.h"
#include "../infrastructure/result.h"
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
  Result<void> sendRegister(uint8_t regNumber, uint16_t value) override {
    try {
      modbus.writeRegister(MODBUS_BROADCAST, modbus.regAddress(regNumber), value);
      Serial.printf("[REPO-IMPL] sendRegister(REG%d, 0x%04X) OK\n", regNumber, value);
      return Result<void>::Ok();
    } catch (...) {
      Serial.printf("[REPO-IMPL] sendRegister(REG%d) FAILED\n", regNumber);
      return Result<void>::Err(ErrorCode::COMMUNICATION_ERROR, "Modbus transmission failed");
    }
  }

  /**
   * Implementazione: invia tutti e tre i registri
   */
  Result<void> sendAllRegisters(uint16_t regConfig, uint16_t regTemp, uint16_t regMode) override {
    try {
      Serial.println("[REPO-IMPL] Sending all registers...");

      Result<void> r1 = sendRegister(101, regConfig);
      if (r1.isErr()) return r1;

      Result<void> r2 = sendRegister(102, regTemp);
      if (r2.isErr()) return r2;

      Result<void> r3 = sendRegister(103, regMode);
      if (r3.isErr()) return r3;

      Serial.printf("[REPO-IMPL] All registers sent: 0x%04X 0x%04X 0x%04X\n",
                    regConfig, regTemp, regMode);
      return Result<void>::Ok();
    } catch (...) {
      Serial.println("[REPO-IMPL] sendAllRegisters FAILED");
      return Result<void>::Err(ErrorCode::COMMUNICATION_ERROR, "Send all registers failed");
    }
  }

  /**
   * Implementazione: legge un registro (placeholder per future espansioni)
   */
  Result<uint16_t> readRegister(uint8_t regNumber) override {
    // Implementazione futura per leggere stato dal dispositivo
    Serial.printf("[REPO-IMPL] readRegister(REG%d) - NOT IMPLEMENTED\n", regNumber);
    return Result<uint16_t>::Err(ErrorCode::NOT_FOUND, "Register read not implemented");
  }
};

#endif
