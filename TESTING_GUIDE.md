# Unit Testing Guide — Viessmann Clean Architecture

## 🎯 Cosa Puoi Testare

Con **MockRepository**, puoi testare:
- ✅ **Logica di business pura** (Use Cases)
- ✅ **Validazione** (temperature clamping, fan speed validation)
- ✅ **Comunicazione** (quali registri vengono inviati)
- ✅ **Error handling** (cosa succede se Repository fallisce)

**Senza:**
- ❌ Hardware Modbus
- ❌ Display LVGL
- ❌ WiFi
- ❌ Dipendenze esterne

## 🏗️ Architettura di Testing

```
┌────────────────────────────────┐
│  Unit Test (test_viessmann.cpp)│
│  Usa MockRepository            │
└────────────────┬───────────────┘
                 │
┌────────────────▼───────────────┐
│  Use Cases (use_cases_v2.h)    │
│  Logica pura, testabile        │
└────────────────┬───────────────┘
                 │
┌────────────────▼───────────────┐
│  MockRepository (mock_*.h)     │
│  Finto repository per test     │
│  Implementa IViessmannRepository
└────────────────────────────────┘
```

## 📋 File di Test

**Ubicazione:**
```
src/
└── test/
    ├── mock_repository.h        # MockRepository
    └── test_viessmann.cpp       # Unit tests
```

## 🚀 Come Eseguire i Test

### Opzione 1: Compilare e Eseguire (Linux/Mac)

```bash
cd /Users/francesco/Documents/Altri\ progetti/viessmann-waveshare-s3

# Compila i test (standalone, NON su Arduino)
g++ -I. -I./src src/test/test_viessmann.cpp -o test_viessmann -std=c++11

# Esegui
./test_viessmann
```

**Output:**
```
▶ Test 1: SetTemperatureUseCase - Valid Temperature
  ✓ Register number should be 102
  ✓ Register value should be 0x00E1 (225)
  ✓ sendRegister should be called once

▶ Test 2: SetTemperatureUseCase - Clamp Minimum (5°C)
  ✓ Temperature should be clamped to 5.0°C (50)

...

╔════════════════════════════════╗
║ TEST RESULTS                   ║
╠════════════════════════════════╣
║ Total:  11                     ║
║ Passed: 11                     ║
║ Failed: 0                      ║
╚════════════════════════════════╝

✅ ALL TESTS PASSED!
```

### Opzione 2: Integrazione con PlatformIO

Aggiungi a `platformio.ini`:

```ini
[env:test]
platform = native
test_framework = googletest
test_dir = src/test
```

Poi:
```bash
pio test -e test
```

## 🧪 Anatomia di un Test

```cpp
void test_setTemperatureUseCase_validTemperature() {
  test_begin("SetTemperatureUseCase - Valid Temperature");

  // 1. Setup: Crea MockRepository e Use Case
  MockRepository mockRepo;
  ViessmannModel model;
  SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo);

  // 2. Execute: Chiama il metodo da testare
  uc.execute(22.5);

  // 3. Assert: Verifica i risultati
  uint16_t expectedReg = (uint16_t)(22.5 * 10);
  test_assert(mockRepo.history.lastRegNumber == 102,
              "Register number should be 102");
  test_assert(mockRepo.history.lastRegValue == expectedReg,
              "Register value should be 0x00E1");
}
```

## 📊 Test Coverage

I test coprono:

| Scenario | Test |
|----------|------|
| **SetTemperatureUseCase** | |
| Temperature valida | `test_setTemperatureUseCase_validTemperature` |
| Clamp minimum (5°C) | `test_setTemperatureUseCase_clampMinimum` |
| Clamp maximum (24°C) | `test_setTemperatureUseCase_clampMaximum` |
| **TogglePowerUseCase** | |
| Power ON | `test_togglePowerUseCase_powerOn` |
| Power OFF | `test_togglePowerUseCase_powerOff` |
| **ChangeHeatingModeUseCase** | |
| Switch to Heating | `test_changeHeatingModeUseCase_toHeating` |
| Switch to Cooling | `test_changeHeatingModeUseCase_toCooling` |
| **ChangeFanSpeedUseCase** | |
| Set valid speed | `test_changeFanSpeedUseCase_setSpeed` |
| Reject invalid speed | `test_changeFanSpeedUseCase_invalidSpeed` |
| **Error Handling** | |
| Repository failure | `test_mockRepository_failureHandling` |
| Multiple operations | `test_multipleOperations` |

## 🔧 Come Aggiungere Nuovi Test

### 1. Aggiungi la funzione test in `test_viessmann.cpp`:

```cpp
void test_myNewFeature() {
  test_begin("My New Feature");

  // Setup
  MockRepository mockRepo;
  ViessmannModel model;
  MyNewUseCase uc(model, (IViessmannRepository&)mockRepo);

  // Execute
  uc.execute(someParam);

  // Assert
  test_assert(mockRepo.history.someValue == expectedValue,
              "Description of what we're testing");
}
```

### 2. Chiama il test in `main()`:

```cpp
int main() {
  // ... existing tests ...
  test_myNewFeature();  // ← Aggiungi qui
  // ...
}
```

### 3. Ricompila ed esegui:

```bash
g++ -I. -I./src src/test/test_viessmann.cpp -o test_viessmann -std=c++11
./test_viessmann
```

## 🎯 MockRepository API

### Tracking dei Comandi

```cpp
MockRepository mockRepo;
// ... esegui test ...

// Leggi cosa è stato inviato
uint8_t regNum = mockRepo.history.lastRegNumber;
uint16_t regVal = mockRepo.history.lastRegValue;
int callCount = mockRepo.history.sendRegisterCallCount;
```

### Simulare Errori

```cpp
MockRepository mockRepo;
mockRepo.history.shouldFailNext = true;  // Prossimo comando fallirà

SetTemperatureUseCase uc(model, (IViessmannRepository&)mockRepo);
uc.execute(20.0);  // Fallirà
```

### Helper Methods

```cpp
mockRepo.assertLastRegister(102, 0x00E1);
mockRepo.assertAllRegisters(0x4003, 0x00CD, 0xb9);
mockRepo.printCallHistory();
mockRepo.reset();  // Cancella history
```

## 🔄 Workflow di Development

```
1. Scrivi il test PRIMA
   ↓
2. Implementa il Use Case
   ↓
3. Esegui il test
   ↓
4. Se fallisce → fix il codice
   ↓
5. Se passa → continua al prossimo test
   ↓
6. Tutti i test verdi? → Ready per hardware!
```

## ✅ Checklist Pre-Deploy

Prima di flashare su Arduino:

- [ ] Tutti i test passano localmente
- [ ] MockRepository è stato usato per validation
- [ ] Error handling è testato
- [ ] Edge cases sono coperti
- [ ] `main_complete.cpp` è compreso e funziona
- [ ] Hardware (Modbus, WiFi) è configurato
- [ ] Test su device fisico confermano il comportamento

## 🚀 Prossimi Step

1. **Esegui i test**:
   ```bash
   ./test_viessmann
   ```

2. **Aggiungi i tuoi test** per nuove feature

3. **Compila il main**:
   ```bash
   pio run --target main_complete.cpp
   ```

4. **Flasha su Arduino** e verifica il comportamento

---

**Ricorda**: MockRepository rende possibile testare la logica senza hardware. È il valore reale della Clean Architecture! 🎯
