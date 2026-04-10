#ifndef SERIAL_LOGGER_H
#define SERIAL_LOGGER_H

#include "./i_logger.h"

#ifdef ARDUINO
  #include <Arduino.h>
#endif

/**
 * SerialLogger - Logs to Arduino Serial
 *
 * Use this in main_complete.cpp for actual device.
 */

class SerialLogger : public ILogger {
public:
  void info(const char* message) override {
    #ifdef ARDUINO
      Serial.printf("[INFO] %s\n", message);
    #else
      printf("[INFO] %s\n", message);
    #endif
  }

  void error(const char* message) override {
    #ifdef ARDUINO
      Serial.printf("[ERROR] %s\n", message);
    #else
      printf("[ERROR] %s\n", message);
    #endif
  }

  void debug(const char* message) override {
    #ifdef ARDUINO
      Serial.printf("[DEBUG] %s\n", message);
    #else
      printf("[DEBUG] %s\n", message);
    #endif
  }
};

#endif
