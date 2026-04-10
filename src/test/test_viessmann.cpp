/*
 * Unit Tests — Viessmann Use Cases
 *
 * Questi test verificano la logica di business SENZA hardware.
 * Usano MockRepository per simulare la persistenza.
 *
 * Compile & Run:
 *   g++ -I. test_viessmann.cpp -o test_viessmann
 *   ./test_viessmann
 */

#include <cstdio>
#include <cassert>
#include <cstring>

// Import domain
#include "../domain/i_viessmann_repository.h"
#include "../domain/use_cases_v2.h"
#include "../model/viessmann_model_v2.h"
#include "../infrastructure/null_logger.h"
#include "../infrastructure/result.h"
#include "../test/mock_repository.h"

// ========== TEST FRAMEWORK SEMPLICE ==========

int totalTests = 0;
int passedTests = 0;

void test_begin(const char* testName) {
  totalTests++;
  printf("\n▶ Test %d: %s\n", totalTests, testName);
}

void test_assert(bool condition, const char* message) {
  if (condition) {
    printf("  ✓ %s\n", message);
    passedTests++;
  } else {
    printf("  ✗ FAILED: %s\n", message);
  }
}

void test_summary() {
  printf("\n╔════════════════════════════════════╗\n");
  printf("║ TEST RESULTS                       ║\n");
  printf("╠════════════════════════════════════╣\n");
  printf("║ Total:  %d                         ║\n", totalTests);
  printf("║ Passed: %d                         ║\n", passedTests);
  printf("║ Failed: %d                         ║\n", totalTests - passedTests);
  printf("╚════════════════════════════════════╝\n\n");

  if (passedTests == totalTests) {
    printf("✅ ALL TESTS PASSED!\n");
  } else {
    printf("❌ SOME TESTS FAILED!\n");
  }
}

// ========== UNIT TESTS ==========

void test_setTemperatureUseCase_validTemperature() {
  test_begin("SetTemperatureUseCase - Valid Temperature");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Execute
  uc.execute(22.5);

  // Assert
  uint16_t expectedReg = (uint16_t)(22.5 * 10);  // 225 = 0x00E1
  test_assert(mockRepo.history.lastRegNumber == 102,
              "Register number should be 102");
  test_assert(mockRepo.history.lastRegValue == expectedReg,
              "Register value should be 0x00E1 (225)");
  test_assert(mockRepo.history.sendRegisterCallCount == 1,
              "sendRegister should be called once");
}

void test_setTemperatureUseCase_clampMinimum() {
  test_begin("SetTemperatureUseCase - Clamp Minimum (5°C)");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Try to set below minimum
  uc.execute(2.0);  // Should clamp to 5.0

  // Assert
  uint16_t expectedReg = 50;  // 5.0 * 10
  test_assert(mockRepo.history.lastRegValue == expectedReg,
              "Temperature should be clamped to 5.0°C (50)");
}

void test_setTemperatureUseCase_clampMaximum() {
  test_begin("SetTemperatureUseCase - Clamp Maximum (24°C)");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Try to set above maximum
  uc.execute(30.0);  // Should clamp to 24.0

  // Assert
  uint16_t expectedReg = 240;  // 24.0 * 10
  test_assert(mockRepo.history.lastRegValue == expectedReg,
              "Temperature should be clamped to 24.0°C (240)");
}

void test_togglePowerUseCase_powerOn() {
  test_begin("TogglePowerUseCase - Power On");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  TogglePowerUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Execute
  uc.executePowerOn();

  // Assert
  test_assert(mockRepo.history.sendAllRegistersCallCount == 1,
              "sendAllRegisters should be called once");
  test_assert(mockRepo.history.lastRegConfig == 0x4003,
              "REG101 should be 0x4003 (FREDDO MAX)");
  test_assert(mockRepo.history.lastRegTemp == 0x00CD,
              "REG102 should be 0x00CD (20.5°C)");
  test_assert(mockRepo.history.lastRegMode == 0xb9,
              "REG103 should be 0xb9");
}

void test_togglePowerUseCase_powerOff() {
  test_begin("TogglePowerUseCase - Power Off");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  TogglePowerUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Execute
  uc.executePowerOff();

  // Assert
  test_assert(mockRepo.history.sendAllRegistersCallCount == 1,
              "sendAllRegisters should be called once");
  test_assert(mockRepo.history.lastRegConfig == 0x4083,
              "REG101 should be 0x4083 (FREDDO + STANDBY)");
  test_assert(mockRepo.history.lastRegTemp == 0x0032,
              "REG102 should be 0x0032 (5.0°C)");
}

void test_changeHeatingModeUseCase_toHeating() {
  test_begin("ChangeHeatingModeUseCase - Switch to Heating");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  ChangeHeatingModeUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Current config: FREDDO (bit14=1, bit13=0)
  uint16_t currentConfig = 0x4003;

  // Execute
  uc.executeHeating(currentConfig);

  // Assert
  test_assert(mockRepo.history.lastRegNumber == 101,
              "Should send to register 101");
  test_assert((mockRepo.history.lastRegValue & (1 << 13)) != 0,
              "Bit 13 (CALDO) should be set");
  test_assert((mockRepo.history.lastRegValue & (1 << 14)) == 0,
              "Bit 14 (FREDDO) should be cleared");
}

void test_changeHeatingModeUseCase_toCooling() {
  test_begin("ChangeHeatingModeUseCase - Switch to Cooling");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  ChangeHeatingModeUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Current config: CALDO (bit14=0, bit13=1)
  uint16_t currentConfig = 0x2003;

  // Execute
  uc.executeCooling(currentConfig);

  // Assert
  test_assert(mockRepo.history.lastRegNumber == 101,
              "Should send to register 101");
  test_assert((mockRepo.history.lastRegValue & (1 << 14)) != 0,
              "Bit 14 (FREDDO) should be set");
  test_assert((mockRepo.history.lastRegValue & (1 << 13)) == 0,
              "Bit 13 (CALDO) should be cleared");
}

void test_changeFanSpeedUseCase_setSpeed() {
  test_begin("ChangeFanSpeedUseCase - Set Fan Speed");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  ChangeFanSpeedUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Current config with FAN=0
  uint16_t currentConfig = 0x4000;

  // Execute: set to FAN MAX (3)
  uc.execute(3, currentConfig);

  // Assert
  test_assert(mockRepo.history.lastRegNumber == 101,
              "Should send to register 101");
  test_assert((mockRepo.history.lastRegValue & 0x03) == 3,
              "Bits 0-1 should be 11 (FAN MAX)");
}

void test_changeFanSpeedUseCase_invalidSpeed() {
  test_begin("ChangeFanSpeedUseCase - Invalid Speed");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  ChangeFanSpeedUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Execute: try to set invalid speed (5)
  uc.execute(5, 0x4000);

  // Assert
  test_assert(mockRepo.history.sendRegisterCallCount == 0,
              "Should NOT send register for invalid speed");
}

void test_mockRepository_failureHandling() {
  test_begin("MockRepository - Failure Handling");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo, &logger);

  // Simulate failure
  mockRepo.history.shouldFailNext = true;

  // Execute (should handle failure gracefully)
  uc.execute(20.0);

  // Assert
  test_assert(mockRepo.history.sendRegisterCallCount == 1,
              "sendRegister should still be called");
  // (The Use Case should print error but not crash)
}

void test_multipleOperations() {
  test_begin("Multiple Operations in Sequence");

  MockRepository mockRepo;
  ViessmannModel model;
  NullLogger logger;
  SetTemperatureUseCase tempUC(model, (IViessmannRepository&)mockRepo, &logger);
  TogglePowerUseCase powerUC(model, (IViessmannRepository&)mockRepo, &logger);
  ChangeHeatingModeUseCase modeUC(model, (IViessmannRepository&)mockRepo, &logger);

  // Sequence: Power ON → Set Temp → Change Mode
  powerUC.executePowerOn();
  mockRepo.reset();

  tempUC.execute(18.0);
  test_assert(mockRepo.history.lastRegValue == 180,
              "Temp should be 18.0°C");
  mockRepo.reset();

  modeUC.executeHeating(0x4003);
  test_assert((mockRepo.history.lastRegValue & (1 << 13)) != 0,
              "Should switch to heating");
}

// ========== MAIN TEST RUNNER ==========

int main() {
  printf("\n╔═════════════════════════════════════╗\n");
  printf("║  Viessmann Clean Architecture      ║\n");
  printf("║  Unit Tests with Mock Repository   ║\n");
  printf("╚═════════════════════════════════════╝\n");

  // Run all tests
  test_setTemperatureUseCase_validTemperature();
  test_setTemperatureUseCase_clampMinimum();
  test_setTemperatureUseCase_clampMaximum();
  test_togglePowerUseCase_powerOn();
  test_togglePowerUseCase_powerOff();
  test_changeHeatingModeUseCase_toHeating();
  test_changeHeatingModeUseCase_toCooling();
  test_changeFanSpeedUseCase_setSpeed();
  test_changeFanSpeedUseCase_invalidSpeed();
  test_mockRepository_failureHandling();
  test_multipleOperations();

  // Print summary
  test_summary();

  return (passedTests == totalTests) ? 0 : 1;
}

/*
 * ═════════════════════════════════════════════════════════════
 * HOW TO RUN:
 *
 * 1. Compila i test (senza hardware):
 *    g++ -I. src/test/test_viessmann.cpp -o test_viessmann -std=c++11
 *
 * 2. Esegui:
 *    ./test_viessmann
 *
 * 3. Output:
 *    ✓ All tests passed!
 *
 * VANTAGGI:
 * ✅ Testi senza hardware (MockRepository)
 * ✅ Verifiche logica di business isolata
 * ✅ Veloce da eseguire (niente Modbus)
 * ✅ Riproducibile (niente variabilità hardware)
 * ═════════════════════════════════════════════════════════════
 */
