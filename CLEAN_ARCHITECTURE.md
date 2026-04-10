# Clean Architecture — VISLA Viessmann Controller

## 📐 Struttura del Progetto

```
src/
├── main_clean_arch.cpp           # Entry point (Orchestrazione)
│
├── devices/                       # LAYER 0: Devices & Hardware
│   └── modbus_service.h          # RS485 communication
│
├── data/                          # LAYER 1: Data Access (Repository)
│   └── viessmann_repository.h    # Repository pattern (astrae Modbus)
│
├── domain/                        # LAYER 2: Business Logic (Use Cases)
│   └── use_cases.h               # SetTemperature, TogglePower, etc
│
├── model/                         # LAYER 3: Entity State
│   ├── viessmann_model_v2.h      # Model con dependency injection
│   └── modbus_service.h          # (devices)
│
├── viewmodel/                     # LAYER 4: Presentation Logic
│   └── viessmann_viewmodel.h     # ViewModel (input/output trasformazione)
│
└── view/                          # LAYER 5: UI
    └── viessmann_view.h          # LVGL interface
```

## 🎯 Architettura — Cerchi Concentrici

```
                    ┌─────────────────────────────────┐
                    │  View (LVGL)                    │
                    │  - Widget creation              │
                    │  - Event forwarding             │
                    └────────────┬────────────────────┘
                                 │
                    ┌────────────▼──────────────────┐
                    │  ViewModel (Presentation)     │
                    │  - Data transformation        │
                    │  - Input handling             │
                    └────────────┬──────────────────┘
                                 │
                    ┌────────────▼──────────────────┐
                    │  Use Cases (Domain Logic)     │
                    │  - SetTemperatureUseCase      │
                    │  - TogglePowerUseCase         │
                    │  - ChangeHeatingModeUseCase   │
                    │  - ChangeFanSpeedUseCase      │
                    └────────────┬──────────────────┘
                                 │
                    ┌────────────▼──────────────────┐
                    │  Model (Entity State)         │
                    │  - regConfig, regTemp, regMode│
                    │  - powerOn, heating           │
                    └────────────┬──────────────────┘
                                 │
                    ┌────────────▼──────────────────┐
                    │  Repository (Data Access)     │
                    │  - sendRegister()             │
                    │  - sendAllRegisters()         │
                    └────────────┬──────────────────┘
                                 │
                    ┌────────────▼──────────────────┐
                    │  ModbusService (Hardware)     │
                    │  - RS485 communication        │
                    │  - Frame building             │
                    │  - LRC calculation            │
                    └──────────────────────────────┘
```

## 🔄 Flusso di un Comando

### Esempio: Utente clicca "Temperatura +"

```
1. View (LVGL)
   ↓ User taps btn_temp_up
   btn_temp_up_callback() → viewModel.onTemperatureUp()

2. ViewModel (Presentation)
   ↓
   onTemperatureUp() → model.increaseTemperature()

3. Model (Entity)
   ↓
   increaseTemperature() → setTemperature(newTemp)
   setTemperature() → setTempUC.execute(newTemp)  [injected UseCase]

4. UseCase (Domain)
   ↓
   SetTemperatureUseCase.execute():
   - Validazione: if (temp < 5.0) temp = 5.0
   - Calcolo: regValue = temp * 10
   - Persistenza: repository.sendRegister(102, regValue)

5. Repository (Data Access)
   ↓
   sendRegister() → modbus.writeRegister(addr, reg, value)

6. ModbusService (Hardware)
   ↓
   writeRegister():
   - Crea frame Modbus ASCII
   - Calcola LRC
   - Invia su RS485

7. Notifica di Cambio (Observer Pattern)
   ↓ UseCase completo
   Model.notifyChanged() → ViewModel.onStateChanged()
   ↓ ViewModel trasforma dati
   ViewModel.updateDisplay() → View.refreshDisplay()
   ↓ View aggiorna LVGL
   lv_label_set_text(label_temp, "20.5°C")
```

## 📦 Dependency Injection nel main

```cpp
// Layer 0: Devices
ModbusService modbus;

// Layer 1: Data
ViessmannRepository repository(modbus);

// Layer 3: Model
ViessmannModel model;

// Layer 2: Use Cases (inject Model + Repository)
SetTemperatureUseCase setTempUC(model, repository);
TogglePowerUseCase togglePowerUC(model, repository);
ChangeHeatingModeUseCase changeModeUC(model, repository);
ChangeFanSpeedUseCase changeFanUC(model, repository);

// Layer 4: ViewModel
ViessmannViewModel viewModel(model);

// Layer 5: View
ViessmannView view(viewModel);

// In setup():
model.injectUseCases(&setTempUC, &togglePowerUC, &changeModeUC, &changeFanUC);
```

## ✅ Vantaggi di questa Architettura

### 🎯 Testabilità
```cpp
// Testa UseCase senza Model
void test_setTemperature() {
  MockRepository mockRepo;
  MockModel mockModel;
  
  SetTemperatureUseCase uc(mockModel, mockRepo);
  uc.execute(22.5);
  
  assert(mockRepo.lastSentRegister == 0x00E1);  // 22.5 * 10
}
```

### 🎯 Indipendenza dai Framework
- Model NON conosce LVGL
- Use Cases NON conoscono Modbus
- Stessa logica può girare su CLI, web, mobile

### 🎯 Manutenibilità
- Aggiungi feature? Aggiungi un UseCase
- Cambi hardware? Modifica solo Repository
- Cambi UI? Modifica solo View

### 🎯 Scalabilità
```cpp
// Nuova feature facile:
class IncreaseAllTemperaturesUseCase {
  // Aumenta temp di tutti i ventilconvettori
};

// Aggiungi al main, fatto!
```

## 🚀 Come Usare

### 1. Compila con main_clean_arch.cpp
```bash
pio run -t build --target main_clean_arch.cpp
```

### 2. API REST
```bash
# Get status
curl http://<ip>/status

# Set temperature
curl -X POST http://<ip>/temperature?value=22.5

# Power on/off
curl -X POST http://<ip>/power/on
curl -X POST http://<ip>/power/off

# Set mode
curl -X POST http://<ip>/mode?mode=heat
curl -X POST http://<ip>/mode?mode=cool

# Set fan speed
curl -X POST http://<ip>/fan?speed=3
```

### 3. LVGL Touch
- Tap "+" button per aumentare temperatura
- Tap "-" button per diminuire
- Tap "ON"/"OFF" per potenza
- Tap "🔥 HEAT"/"❄️ COOL" per modalità

## 📋 Responsabilità di Ogni Layer

| Layer | Responsabilità | Non Deve Conoscere |
|-------|----------------|--------------------|
| **View** | Render LVGL, event callbacks | Business logic |
| **ViewModel** | Data transformation, orchestration | LVGL details, Modbus |
| **Use Cases** | Business logic, validazione | UI, hardware |
| **Model** | Entity state, observers | UI, database |
| **Repository** | Data access abstraction | Modbus details |
| **ModbusService** | RS485 communication | Business logic |

## 🔧 Aggiungere un Nuovo Use Case

### 1. Definisci in `domain/use_cases.h`
```cpp
class MyNewUseCase {
private:
  ViessmannModel &model;
  ViessmannRepository &repository;

public:
  MyNewUseCase(ViessmannModel &m, ViessmannRepository &r)
      : model(m), repository(r) {}

  void execute(float param) {
    // Logica di business
    // Accesso a repository per persistenza
  }
};
```

### 2. Aggiungi a Model
```cpp
void injectUseCases(
    // ... existing ...
    MyNewUseCase *myNew) {
  myNewUC = myNew;
}
```

### 3. Istanzia nel main
```cpp
MyNewUseCase myNewUC(model, repository);
model.injectUseCases(&setTempUC, ..., &myNewUC);
```

### 4. Chiama da ViewModel
```cpp
class ViessmannViewModel {
  void onMyNewInput() {
    model.myNewMethod();  // che a sua volta chiama usecase
  }
};
```

## 📚 Ulteriori Risorse

- **Uncle Bob's Clean Architecture**: https://blog.cleancoder.com/
- **Dependency Injection in C++**: Pattern senza framework
- **Observer Pattern**: usato per state notifications
- **Repository Pattern**: astrazione data access

## 🎓 Principi SOLID Applicati

- **S**ingle Responsibility: Ogni classe ha UNA responsabilità
- **O**pen/Closed: Aperto per estensione, chiuso per modifica
- **L**iskov Substitution: Repository potrebbe essere swappato
- **I**nterface Segregation: Interfacce piccole e focused
- **D**ependency Inversion: Dipendi da astrazioni, non da implementazioni

---

**Versione**: 3.0 (Use Cases + Repository)  
**Data**: Aprile 2026  
**Autore**: Clean Architecture Refactor
