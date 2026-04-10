#ifndef MOCK_REPOSITORY_H
#define MOCK_REPOSITORY_H

#include "../domain/i_viessmann_repository.h"
#include "../infrastructure/result.h"
#include <cstdint>
#include <cstdio>

/**
 * MockRepository - Per Testing Senza Hardware
 *
 * Implementa IViessmannRepository ma NON parla a Modbus.
 * Usa questa per testare Use Cases senza hardware.
 *
 * IMPORTANTE: Questa dimostra il VALORE di Dependency Inversion!
 * Puoi swappare ViessmannRepositoryImpl con MockRepository
 * e i Use Cases funzionano identicamente.
 *
 * Esempio di uso:
 *
 *   MockRepository mockRepo;
 *   ViessmannModel model;
 *   SetTemperatureUseCase uc(model, mockRepo);
 *
 *   // Test senza Modbus, WiFi, o qualsiasi hardware!
 *   uc.execute(22.5);
 *   assert(mockRepo.lastSentRegister == 0x00E1);  // 22.5 * 10
 */

class MockRepository : public IViessmannRepository {
public:
  // Variabili per tracking dei comandi ricevuti
  struct CallHistory {
    uint8_t lastRegNumber = 0;
    uint16_t lastRegValue = 0;
    uint16_t lastRegConfig = 0;
    uint16_t lastRegTemp = 0;
    uint16_t lastRegMode = 0;
    int sendRegisterCallCount = 0;
    int sendAllRegistersCallCount = 0;
    int readRegisterCallCount = 0;
    bool shouldFailNext = false;
  } history;

  ~MockRepository() = default;

  // ========== IMPLEMENTAZIONE INTERFACE ==========

  Result<void> sendRegister(uint8_t regNumber, uint16_t value) override {
    history.lastRegNumber = regNumber;
    history.lastRegValue = value;
    history.sendRegisterCallCount++;

    if (history.shouldFailNext) {
      history.shouldFailNext = false;
      printf("[MOCK] sendRegister(REG%d, 0x%04X) → FAIL\n", regNumber, value);
      return Result<void>::Err(
          ErrorCode::COMMUNICATION_ERROR,
          "Mock failure: sendRegister failed"
      );
    }

    printf("[MOCK] sendRegister(REG%d, 0x%04X) → OK\n", regNumber, value);
    return Result<void>::Ok();
  }

  Result<void> sendAllRegisters(uint16_t regConfig, uint16_t regTemp, uint16_t regMode) override {
    history.lastRegConfig = regConfig;
    history.lastRegTemp = regTemp;
    history.lastRegMode = regMode;
    history.sendAllRegistersCallCount++;

    if (history.shouldFailNext) {
      history.shouldFailNext = false;
      printf("[MOCK] sendAllRegisters(0x%04X, 0x%04X, 0x%04X) → FAIL\n",
             regConfig, regTemp, regMode);
      return Result<void>::Err(
          ErrorCode::COMMUNICATION_ERROR,
          "Mock failure: sendAllRegisters failed"
      );
    }

    printf("[MOCK] sendAllRegisters(0x%04X, 0x%04X, 0x%04X) → OK\n",
           regConfig, regTemp, regMode);
    return Result<void>::Ok();
  }

  Result<uint16_t> readRegister(uint8_t regNumber) override {
    history.readRegisterCallCount++;
    printf("[MOCK] readRegister(REG%d) → 0x0000\n", regNumber);
    return Result<uint16_t>::Ok(0);
  }

  // ========== TEST HELPERS ==========

  void assertLastRegister(uint8_t expectedReg, uint16_t expectedValue) {
    if (history.lastRegNumber != expectedReg || history.lastRegValue != expectedValue) {
      printf("❌ ASSERT FAILED!\n");
      printf("   Expected: REG%d = 0x%04X\n", expectedReg, expectedValue);
      printf("   Got:      REG%d = 0x%04X\n", history.lastRegNumber, history.lastRegValue);
    } else {
      printf("✓ ASSERT PASSED: REG%d = 0x%04X\n", expectedReg, expectedValue);
    }
  }

  void assertAllRegisters(uint16_t expectedConfig, uint16_t expectedTemp, uint16_t expectedMode) {
    bool pass = (history.lastRegConfig == expectedConfig &&
                 history.lastRegTemp == expectedTemp &&
                 history.lastRegMode == expectedMode);

    if (!pass) {
      printf("❌ ASSERT FAILED!\n");
      printf("   Expected: 0x%04X 0x%04X 0x%04X\n", expectedConfig, expectedTemp, expectedMode);
      printf("   Got:      0x%04X 0x%04X 0x%04X\n",
             history.lastRegConfig, history.lastRegTemp, history.lastRegMode);
    } else {
      printf("✓ ASSERT PASSED: All registers match\n");
    }
  }

  void reset() {
    history = {};
  }

  void printCallHistory() {
    printf("\n[MOCK] Call History:\n");
    printf("  sendRegister calls:      %d\n", history.sendRegisterCallCount);
    printf("  sendAllRegisters calls:  %d\n", history.sendAllRegistersCallCount);
    printf("  readRegister calls:      %d\n", history.readRegisterCallCount);
    printf("  Last command: REG%d = 0x%04X\n", history.lastRegNumber, history.lastRegValue);
    printf("\n");
  }
};

/*
 * ═══════════════════════════════════════════════════════════════
 * ESEMPIO DI UNIT TEST CON MOCK REPOSITORY
 * ═══════════════════════════════════════════════════════════════
 *
 * #include "test/mock_repository.h"
 * #include "domain/use_cases_v2.h"
 * #include "model/viessmann_model_v2.h"
 *
 * void test_setTemperatureUseCase() {
 *   printf("\n▶ Testing SetTemperatureUseCase\n");
 *
 *   // Setup
 *   MockRepository mockRepo;
 *   ViessmannModel model;
 *   SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo);
 *
 *   // Execute
 *   uc.execute(22.5);
 *
 *   // Assert
 *   uint16_t expectedReg = (uint16_t)(22.5 * 10);  // 0x00E1
 *   mockRepo.assertLastRegister(102, expectedReg);
 *   mockRepo.printCallHistory();
 * }
 *
 * void test_togglePowerUseCase() {
 *   printf("\n▶ Testing TogglePowerUseCase\n");
 *
 *   MockRepository mockRepo;
 *   ViessmannModel model;
 *   TogglePowerUseCase uc(model, (IViessmannRepository&)mockRepo);
 *
 *   // Test Power On
 *   uc.executePowerOn();
 *   mockRepo.assertAllRegisters(0x4003, 0x00CD, 0xb9);
 *
 *   // Test Power Off
 *   mockRepo.reset();
 *   uc.executePowerOff();
 *   mockRepo.assertAllRegisters(0x4083, 0x0032, 0xb9);
 * }
 *
 * void test_repository_failure() {
 *   printf("\n▶ Testing Repository Failure Handling\n");
 *
 *   MockRepository mockRepo;
 *   ViessmannModel model;
 *   SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo);
 *
 *   // Simula errore repository
 *   mockRepo.history.shouldFailNext = true;
 *   uc.execute(20.0);  // Fallirà
 * }
 *
 * ═══════════════════════════════════════════════════════════════
 */

#endif
