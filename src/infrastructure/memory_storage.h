#ifndef MEMORY_STORAGE_H
#define MEMORY_STORAGE_H

#include "./i_storage.h"
#include <map>

/**
 * MemoryStorage — In-Memory Storage for Testing
 *
 * Implementazione volatile di IStorage.
 * Dati in memoria RAM, NON persistono al reboot.
 *
 * Uso:
 * - Unit testing (MockRepository con MemoryStorage)
 * - Prototipazione veloce
 * - Ambienti dove Flash non è disponibile
 *
 * Thread-safety: NO (non necessaria per test embedded)
 */

class MemoryStorage : public IStorage {
private:
  // Registri Modbus
  uint16_t temp = 0xCD;      // Default: 20.5°C
  uint16_t config = 0x4003;  // Default: FREDDO MAX ON
  uint16_t mode = 0xb9;      // Default: seasonal

  // Generic storage
  std::map<const char *, uint32_t> uint32Map;
  std::map<const char *, float> floatMap;

public:
  MemoryStorage() = default;

  // ========== Temperature ==========

  Result<uint16_t> readTemperature() override {
    return Result<uint16_t>::Ok(temp);
  }

  Result<void> writeTemperature(uint16_t value) override {
    if (value < 0x32 || value > 0x190) {
      return Result<void>::Err(
          ErrorCode::VALIDATION_FAILED,
          "Temperature out of range"
      );
    }
    temp = value;
    return Result<void>::Ok();
  }

  // ========== Config ==========

  Result<uint16_t> readConfig() override {
    return Result<uint16_t>::Ok(config);
  }

  Result<void> writeConfig(uint16_t value) override {
    config = value;
    return Result<void>::Ok();
  }

  // ========== Mode ==========

  Result<uint16_t> readMode() override {
    return Result<uint16_t>::Ok(mode);
  }

  Result<void> writeMode(uint16_t value) override {
    mode = value;
    return Result<void>::Ok();
  }

  // ========== Generic uint32 ==========

  Result<uint32_t> readUInt32(const char *key) override {
    if (uint32Map.find(key) != uint32Map.end()) {
      return Result<uint32_t>::Ok(uint32Map[key]);
    }
    return Result<uint32_t>::Err(
        ErrorCode::NOT_FOUND,
        "Key not found"
    );
  }

  Result<void> writeUInt32(const char *key, uint32_t value) override {
    uint32Map[key] = value;
    return Result<void>::Ok();
  }

  // ========== Generic float ==========

  Result<float> readFloat(const char *key) override {
    if (floatMap.find(key) != floatMap.end()) {
      return Result<float>::Ok(floatMap[key]);
    }
    return Result<float>::Err(
        ErrorCode::NOT_FOUND,
        "Key not found"
    );
  }

  Result<void> writeFloat(const char *key, float value) override {
    floatMap[key] = value;
    return Result<void>::Ok();
  }

  // ========== Utility ==========

  Result<void> clear() override {
    temp = 0xCD;
    config = 0x4003;
    mode = 0xb9;
    uint32Map.clear();
    floatMap.clear();
    return Result<void>::Ok();
  }

  Result<void> commit() override {
    // Memory storage is always committed
    return Result<void>::Ok();
  }
};

#endif
