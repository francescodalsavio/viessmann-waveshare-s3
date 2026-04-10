#ifndef I_LOGGER_H
#define I_LOGGER_H

/**
 * ILogger - Abstract Logger Interface
 *
 * Domain layer can optionally log through this interface.
 * This keeps the domain layer framework-agnostic and testable.
 *
 * Implementations:
 * - SerialLogger: Logs to Arduino Serial
 * - NullLogger: No-op logger for tests
 */

class ILogger {
public:
  virtual ~ILogger() = default;

  virtual void info(const char* message) = 0;
  virtual void error(const char* message) = 0;
  virtual void debug(const char* message) = 0;
};

#endif
