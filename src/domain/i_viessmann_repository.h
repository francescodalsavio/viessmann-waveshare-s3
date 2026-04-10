#ifndef I_VIESSMANN_REPOSITORY_H
#define I_VIESSMANN_REPOSITORY_H

#include <cstdint>
#include "../infrastructure/result.h"

/**
 * IViessmannRepository - Abstract Interface (Domain Layer)
 *
 * Questa è l'INTERFACE che il Domain definisce.
 * La Data Layer (Repository concreto) implementa questa interface.
 *
 * Vantaggi:
 * ✅ Use Cases dipendono dall'interface, NON dall'implementazione
 * ✅ Puoi mockare per testing
 * ✅ Puoi swappare implementazione senza cambiare Use Cases
 * ✅ True Dependency Inversion (SOLID - D)
 * ✅ Result<T> per error handling type-safe
 *
 * Clean Architecture: Il Domain DEFINISCE cosa serve (interface)
 *                     Data Layer IMPLEMENTA come farlo
 */

class IViessmannRepository {
public:
  virtual ~IViessmannRepository() = default;

  /**
   * Invia un singolo registro al dispositivo
   * @param regNumber Numero registro (101, 102, 103)
   * @param value Valore a 16-bit
   * @return Result<bool> con esito operazione
   */
  virtual Result<void> sendRegister(uint8_t regNumber, uint16_t value) = 0;

  /**
   * Invia tutti e tre i registri
   * @param regConfig REG 101 (configuration)
   * @param regTemp REG 102 (temperature)
   * @param regMode REG 103 (mode)
   * @return Result<void> con esito operazione
   */
  virtual Result<void> sendAllRegisters(uint16_t regConfig, uint16_t regTemp, uint16_t regMode) = 0;

  /**
   * Legge lo stato corrente dai registri (per future espansioni)
   * @return Result<uint16_t> con valore letto
   */
  virtual Result<uint16_t> readRegister(uint8_t regNumber) = 0;
};

#endif
