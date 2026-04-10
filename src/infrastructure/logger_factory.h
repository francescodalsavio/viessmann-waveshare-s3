#ifndef LOGGER_FACTORY_H
#define LOGGER_FACTORY_H

#include "./i_logger.h"
#include "./serial_logger.h"
#include "./null_logger.h"

/**
 * LoggerFactory — Central Logger Management
 *
 * Fornisce istanza globale di logger con singleton pattern.
 * Usato da Repository, Use Cases, Model per logging centralizzato.
 *
 * Vantaggi:
 * ✅ Istanza unica globale (non molteplici istanze)
 * ✅ Configurable in un posto (main)
 * ✅ Swap fra SerialLogger (prod) e NullLogger (test)
 * ✅ Logging centralizzato in tutta l'app
 *
 * Uso:
 *
 * Produzione (main_complete.cpp):
 *   LoggerFactory::setLogger(new SerialLogger());
 *
 * Test:
 *   LoggerFactory::setLogger(new NullLogger());
 *
 * Ovunque nel codice:
 *   ILogger *logger = LoggerFactory::instance();
 *   logger->info("Something happened");
 */

class LoggerFactory {
private:
  static ILogger *globalLogger;

public:
  /**
   * Imposta istanza logger globale
   * Tipicamente chiamato una sola volta in main()
   */
  static void setLogger(ILogger *logger) {
    globalLogger = logger;
  }

  /**
   * Ottiene istanza logger globale
   * Se non è stata settata, ritorna NullLogger (no-op)
   */
  static ILogger *instance() {
    if (globalLogger == nullptr) {
      // Fallback: no-op logger se non configurato
      static NullLogger nullLogger;
      return &nullLogger;
    }
    return globalLogger;
  }

  /**
   * Reset (per testing, es: cambiare logger tra test)
   */
  static void reset() {
    globalLogger = nullptr;
  }
};

// Inizializzazione statica
ILogger *LoggerFactory::globalLogger = nullptr;

#endif
