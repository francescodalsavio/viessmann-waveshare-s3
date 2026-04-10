#ifndef I_STORAGE_H
#define I_STORAGE_H

#include "./result.h"

/**
 * IStorage — Abstract Persistent Storage Interface
 *
 * Responsabilità:
 * - Salva setpoint temperatura
 * - Salva preferenze (modalità heat/cool, fan speed, ecc)
 * - Carica dati al boot
 *
 * Implementazioni:
 * - FlashStorage (ESP32 Preferences)
 * - EEPROM Storage (Arduino standard)
 * - MemoryStorage (test, volatile)
 *
 * Vantaggio di Dependency Inversion:
 * Domain layer NON conosce che è Flash/EEPROM/Memory
 * Repository dipende da IStorage interface
 */

class IStorage {
public:
  virtual ~IStorage() = default;

  // Temperature (REG 102)
  virtual Result<uint16_t> readTemperature() = 0;
  virtual Result<void> writeTemperature(uint16_t value) = 0;

  // Config (REG 101)
  virtual Result<uint16_t> readConfig() = 0;
  virtual Result<void> writeConfig(uint16_t value) = 0;

  // Mode (REG 103)
  virtual Result<uint16_t> readMode() = 0;
  virtual Result<void> writeMode(uint16_t value) = 0;

  // Generic key-value storage
  virtual Result<uint32_t> readUInt32(const char *key) = 0;
  virtual Result<void> writeUInt32(const char *key, uint32_t value) = 0;

  virtual Result<float> readFloat(const char *key) = 0;
  virtual Result<void> writeFloat(const char *key, float value) = 0;

  // Utility
  virtual Result<void> clear() = 0;
  virtual Result<void> commit() = 0;
};

#endif
