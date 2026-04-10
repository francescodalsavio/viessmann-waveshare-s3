#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include "./i_storage.h"

#ifdef ARDUINO
  #include <Preferences.h>
#endif

/**
 * FlashStorage — ESP32 Preferences Implementation
 *
 * Salva dati in Flash (NVS - Non-Volatile Storage) di ESP32.
 * I dati persistono anche dopo power-off.
 *
 * Organizzazione:
 * - Namespace "viessmann" isolato da altre app
 * - Keys: "temp", "config", "mode" per registri Modbus
 * - Keys generici per estensioni future
 *
 * Vantaggi:
 * ✅ Dati persistono al reboot
 * ✅ Automatico su ESP32 (Preferences)
 * ✅ Facile aggiungere nuove preferenze
 * ✅ Non interfere con firmware storage
 */

class FlashStorage : public IStorage {
private:
#ifdef ARDUINO
  Preferences preferences;
  const char *NAMESPACE = "viessmann";
#endif

public:
  FlashStorage() {
#ifdef ARDUINO
    preferences.begin(NAMESPACE, false);  // false = read-write mode
#endif
  }

  ~FlashStorage() {
#ifdef ARDUINO
    preferences.end();
#endif
  }

  // ========== Temperature (REG 102) ==========

  Result<uint16_t> readTemperature() override {
#ifdef ARDUINO
    uint16_t value = preferences.getUShort("temp", 0xCD);  // Default: 20.5°C
    return Result<uint16_t>::Ok(value);
#else
    // Test/no-op
    return Result<uint16_t>::Ok(0xCD);
#endif
  }

  Result<void> writeTemperature(uint16_t value) override {
#ifdef ARDUINO
    if (value < 0x32 || value > 0x190) {  // 5°C to 40°C
      return Result<void>::Err(
          ErrorCode::VALIDATION_FAILED,
          "Temperature out of range (0x32-0x190)"
      );
    }
    preferences.putUShort("temp", value);
    return Result<void>::Ok();
#else
    return Result<void>::Ok();
#endif
  }

  // ========== Config (REG 101) ==========

  Result<uint16_t> readConfig() override {
#ifdef ARDUINO
    uint16_t value = preferences.getUShort("config", 0x4003);  // Default: FREDDO MAX ON
    return Result<uint16_t>::Ok(value);
#else
    return Result<uint16_t>::Ok(0x4003);
#endif
  }

  Result<void> writeConfig(uint16_t value) override {
#ifdef ARDUINO
    preferences.putUShort("config", value);
    return Result<void>::Ok();
#else
    return Result<void>::Ok();
#endif
  }

  // ========== Mode (REG 103) ==========

  Result<uint16_t> readMode() override {
#ifdef ARDUINO
    uint16_t value = preferences.getUShort("mode", 0xb9);  // Default: seasonal
    return Result<uint16_t>::Ok(value);
#else
    return Result<uint16_t>::Ok(0xb9);
#endif
  }

  Result<void> writeMode(uint16_t value) override {
#ifdef ARDUINO
    preferences.putUShort("mode", value);
    return Result<void>::Ok();
#else
    return Result<void>::Ok();
#endif
  }

  // ========== Generic Key-Value (uint32) ==========

  Result<uint32_t> readUInt32(const char *key) override {
#ifdef ARDUINO
    uint32_t value = preferences.getUInt(key, 0);
    return Result<uint32_t>::Ok(value);
#else
    (void)key;  // Suppress unused warning
    return Result<uint32_t>::Ok(0);
#endif
  }

  Result<void> writeUInt32(const char *key, uint32_t value) override {
#ifdef ARDUINO
    preferences.putUInt(key, value);
    return Result<void>::Ok();
#else
    (void)key;
    (void)value;
    return Result<void>::Ok();
#endif
  }

  // ========== Generic Key-Value (float) ==========

  Result<float> readFloat(const char *key) override {
#ifdef ARDUINO
    float value = preferences.getFloat(key, 0.0f);
    return Result<float>::Ok(value);
#else
    (void)key;
    return Result<float>::Ok(0.0f);
#endif
  }

  Result<void> writeFloat(const char *key, float value) override {
#ifdef ARDUINO
    preferences.putFloat(key, value);
    return Result<void>::Ok();
#else
    (void)key;
    (void)value;
    return Result<void>::Ok();
#endif
  }

  // ========== Utility ==========

  Result<void> clear() override {
#ifdef ARDUINO
    preferences.clear();
    return Result<void>::Ok();
#else
    return Result<void>::Ok();
#endif
  }

  Result<void> commit() override {
#ifdef ARDUINO
    // Preferences auto-commits, but explicit call is safe
    return Result<void>::Ok();
#else
    return Result<void>::Ok();
#endif
  }
};

#endif
