#ifndef VIESSMANN_REPOSITORY_H
#define VIESSMANN_REPOSITORY_H

#include "../model/modbus_service.h"
#include <cstdint>

/**
 * ViessmannRepository - Data Access Layer
 *
 * Responsabilità:
 * - Astrae la comunicazione hardware (Modbus)
 * - Fornisce interfaccia pulita per accesso ai dati
 * - Potrebbe facilmente essere sostituito (es. WiFi, Ethernet, ecc)
 *
 * Clean Architecture: Layer "Data & Devices"
 * È il confine tra logica pura e hardware.
 */

struct DeviceState {
  uint16_t regConfig;
  uint16_t regTemp;
  uint16_t regMode;
  uint32_t lastUpdateTime;
};

class ViessmannRepository {
private:
  ModbusService &modbus;
  const uint8_t MODBUS_BROADCAST = 0x00;

public:
  ViessmannRepository(ModbusService &mb) : modbus(mb) {}

  /**
   * Invia un registro al dispositivo
   */
  void sendRegister(uint8_t regNumber, uint16_t value) {
    modbus.writeRegister(MODBUS_BROADCAST, modbus.regAddress(regNumber), value);
  }

  /**
   * Invia tutti e tre i registri (config, temp, mode)
   */
  void sendAllRegisters(uint16_t regConfig, uint16_t regTemp, uint16_t regMode) {
    Serial.println("[REPO] Sending all registers...");
    sendRegister(101, regConfig);
    delay(500);
    sendRegister(102, regTemp);
    delay(500);
    sendRegister(103, regMode);
    Serial.printf("[REPO] Sent: 0x%04X 0x%04X 0x%04X\n", regConfig, regTemp, regMode);
  }

  /**
   * Parsa frame Modbus ricevuto
   */
  bool parseFrame(const char* frameStr, ModbusFrame* frame) {
    return modbus.parseFrame(frameStr, frame);
  }
};

#endif
