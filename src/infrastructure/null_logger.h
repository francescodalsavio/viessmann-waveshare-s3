#ifndef NULL_LOGGER_H
#define NULL_LOGGER_H

#include "./i_logger.h"

/**
 * NullLogger - No-op Logger for Tests
 *
 * Use this in unit tests to avoid dependencies on Serial/printf.
 * It implements ILogger but does nothing.
 */

class NullLogger : public ILogger {
public:
  void info(const char* message) override {
    // Do nothing
    (void)message;  // Suppress unused parameter warning
  }

  void error(const char* message) override {
    // Do nothing
    (void)message;
  }

  void debug(const char* message) override {
    // Do nothing
    (void)message;
  }
};

#endif
