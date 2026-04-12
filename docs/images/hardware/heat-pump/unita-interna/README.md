# Vitocal 100-S - Unità Interna (Centralina Idraulica)

**Fonte:** Datasheet tecnico ufficiale Viessmann - Rev. 06/2018

## Panoramica

**Unità interna della Vitocal 100-S** - Centralina idraulica che gestisce tutti i circuiti di riscaldamento, raffrescamento e acqua calda sanitaria. Contiene il **Vitotronic 200** (cervello del sistema).

**IMPORTANTE**: L'unità interna NON include boiler integrato - deve essere collegata a un boiler esterno (es. SPCF 100L).

## Componenti Unità Interna

```
UNITÀ INTERNA VITOCAL 100-S (Centralina Idraulica)
┌────────────────────────────────────────┐
│ 1. FLUSSOSTATO                         │ ← Verifica circolazione acqua
│                                        │
│ 2. CONDENSATORE/EVAPORATORE            │ ← Scambio termico interno
│    (serpentino interno)                │
│                                        │
│ 3. POMPA CIRCOLAZIONE ALTA EFFICIENZA  │ ← Movimenta acqua nei circuiti
│                                        │
│ 4. VALVOLA DEVIATRICE 3-VIE            │ ← Selezione circuito attivo:
│                                        │   • Riscaldamento
│                                        │   • Raffrescamento
│                                        │   • ACS (acqua calda sanitaria)
│                                        │
│ 5. REGOLAZIONE VITOTRONIC 200 ⭐       │ ← CONTROLLO SISTEMA (il cervello!)
│    • Display grafico color             │   • Modbus RTU (RS485)
│    • Letture temperatura               │   • App ViCare (remoto)
│    • Programmi orari                   │   • Regole riscaldamento/ACS
│                                        │
│ COMPONENTI INTEGRATI:                  │
│ • Vaso espansione (protezione p. min)  │
│ • Valvola sicurezza 3 bar              │
│ • Flussostato di protezione            │
└────────────────────────────────────────┘
```

## Funzionamento

### Circuiti Gestiti

```
UNITÀ INTERNA (Centralina Idraulica)
       ↓
Gestisce 3 circuiti indipendenti:

1️⃣ CIRCUITO RISCALDAMENTO (Inverno)
   Boiler esterno (55°C) → Valvola modulante → Fan-coil/Radiatori
   Setpoint: 20-22°C ambiente
   
2️⃣ CIRCUITO RAFFRESCAMENTO (Estate)
   Pompa calore (12-15°C) → Valvola modulante → Fan-coil
   Setpoint: 26-28°C ambiente
   Condensa: Vasca raccolta nei fan-coil
   
3️⃣ CIRCUITO ACS (Acqua Calda Sanitaria)
   Pompa calore integrata in boiler esterno
   Temperatura: 45-50°C comfort (60°C protezione)
   Integrazione solare (pre-riscaldamento)
```

### Modalità Operativa

| Modalità | Funzione | Compressore | Valvola |
|----------|----------|-------------|---------|
| **Pompa Giorno** | Riscaldamento completo | 100% | Riscald. |
| **Pompa Notte** | Modulazione ridotta | 30-70% | Riscald. |
| **Estate** | Raffrescamento | 100% | Raffres. |
| **ACS** | Acqua calda | 100% | ACS |
| **Solare** | Se T_solare > T_boiler | Spento | Bypass |

## Vitotronic 200 - Il Cervello del Sistema

**⭐ IMPORTANTE**: Vitotronic 200 è **sempre presente di fabbrica** nella Vitocal 100-S. NON è separato, NON è opzionale. È il controllore integrato che gestisce TUTTO il sistema.

### Cos'è Vitotronic 200?

Il Vitotronic 200 è una **centralina di controllo dedicata Viessmann** (non un ESP32, non un generico microcontroller). È un dispositivo industriale specializzato che:

- ✅ **Controlla il compressore** della pompa di calore (on/off, modulazione inverter)
- ✅ **Regola le valvole** (riscaldamento, raffrescamento, ACS)
- ✅ **Monitora i sensori** (temperatura mandata, ritorno, boiler, esterno)
- ✅ **Gestisce il circuito solare** (pompa solare, bypasss)
- ✅ **Ha protezioni** (antigelo, surriscaldamento, pressione)
- ✅ **Comunica via Modbus RTU** (RS485) con sistemi esterni come ESP32
- ✅ **Ha un display grafico color** per interfaccia utente locale

### Localizzazione Fisica

Il Vitotronic 200 è **montato DENTRO l'unità interna** (centralina idraulica) in una scatola di controllo. NON è una cosa separata che devi comprare.

```
┌─────────────────────────────────────────────────────┐
│         UNITÀ INTERNA (Centralina Idraulica)        │
│                                                     │
│  ┌─────────────────────────────────────────────┐   │
│  │  SCATOLA CONTROLLO VITOTRONIC 200           │   │
│  │  ├─ Processore Viessmann                    │   │
│  │  ├─ Display grafico color                   │   │
│  │  ├─ Circuiti controllo compressore          │   │
│  │  ├─ Circuiti controllo valvole              │   │
│  │  ├─ Protezioni (antigelo, pressione, etc)   │   │
│  │  ├─ Porta RS485 Modbus RTU (RJ45)           │   │ ← COLLEGAMENTO ESTERNO
│  │  └─ Alimentazione 230V/400V                 │   │
│  └─────────────────────────────────────────────┘   │
│                                                     │
│  Componenti Idraulici Gestiti:                      │
│  • Pompa circolazione                              │
│  • Valvola deviatrice 3-vie                        │
│  • Sensori temperatura                             │
│  • Valvola sicurezza/vaso espansione               │
└─────────────────────────────────────────────────────┘
```

### Come Funziona Vitotronic 200

```
CICLO CONTROLLO (continuo):

1. LETTURA SENSORI
   ├─ T_mandata (temperatura uscita boiler) → Condotto a radiatori/fan-coil
   ├─ T_ritorno (temperatura rientro dal circuito)
   ├─ T_boiler (temperatura accumulatore termico)
   ├─ T_esterna (temperatura aria esterna)
   └─ T_ACS (temperatura acqua calda sanitaria)

2. CALCOLO LOGICA
   ├─ Se T_ambiente < T_setpoint → Attiva riscaldamento
   ├─ Se T_mandata < T_target → Aumenta compressore (inverter)
   ├─ Se T_boiler > T_max → Disattiva compressore
   ├─ Se T_esterna < -5°C → Protezione antigelo
   └─ Se T_solare > T_boiler + 5°C → Attiva pompa solare

3. CONTROLLO ATTUATORI
   ├─ Comando compressore (0-100% velocità via inverter)
   ├─ Comando pompa circolazione (0-100%)
   ├─ Selezione valvola deviatrice (Riscaldamento/Raffrescamento/ACS)
   └─ Controllo fan-coil (velocità ventola)

4. MONITORAGGIO PROTEZIONI
   ├─ Pressione sistema (0,5-1,5 bar ok)
   ├─ Flussostato (circolazione acqua ok)
   └─ Allarmi (visualizza su display)

5. COMUNICAZIONE MODBUS RTU
   └─ Invia dati a ESP32 se collegato via RS485
```

### Display Principale

```
┌──────────────────────────────────────┐
│  VITOTRONIC 200 (Display Grafico)    │
│                                      │
│  Schermata principale mostra:        │
│  • Temperatura attuale (es. 22°C)    │
│  • Temperatura setpoint (es. 20°C)   │
│  • Modalità (Riscaldamento/Raffresc) │
│  • Stato pompa (ON/OFF)              │
│  • Potenza compressore (30%, 80%)    │
│  • Errori/Allarmi (se presenti)      │
│  • Lettura sensori individuali       │
│  • Programmazione oraria             │
│                                      │
│  Navigazione: Pulsanti fisici +/−    │
│  Lingua: Configurabile (default ITA) │
└──────────────────────────────────────┘
```

### Funzioni Vitotronic 200
✅ Regolazione automatica temperatura (PID loop)
✅ Programmazione oraria (pompa giorno/notte, setpoint diversi per ore)
✅ Protezione antigelo (-5°C)
✅ Protezione surriscaldamento (60°C max ACS)
✅ Monitoraggio real-time sensori temperatura
✅ Gestione integrazione solare (pompa solare autonoma)
✅ Comunicazione Modbus RTU (RS485 9600 baud) ← **Per ESP32**
✅ App ViCare (remoto via Vitoconnect, opzionale)

## Specifiche Tecniche Unità Interna

| Parametro | Valore |
|-----------|--------|
| **Dimensioni** | 880 × 450 × 370 mm (compatta) |
| **Vaso espansione** | Integrato (protezione pressione) |
| **Valvola sicurezza** | 3 bar integrata |
| **Flussostato** | Protezione circolazione minima |
| **Pompa circolazione** | Alta efficienza, modulante |
| **Condensatore interno** | Scambio termico gas/acqua |
| **Collegamento gas** | Tubo refrigerante da unità esterna |
| **Collegamento idraulico** | Attacchi 1" per mandata/ritorno |
| **Alimentazione** | 230V monofase o 400V trifase |
| **Potenza assorbimento** | 0,5-2,5 kW (variabile) |

## Collegamenti Unità Interna

```
COLLEGAMENTI FISICI:

Dalla pompa di calore (unità esterna):
├─ Tubo gas refrigerante IN (pressione) → Condensatore
└─ Tubo gas refrigerante OUT (ritorno) ← Condensatore

Alla distribuzione:
├─ Mandata riscaldamento (55°C) → Boiler esterno
├─ Ritorno riscaldamento (45°C) ← Boiler esterno
├─ Mandata ACS → Caldaia/Serbatoio
└─ Ritorno ACS ← Caldaia/Serbatoio

Controllo:
├─ Vitotronic 200 Display (interfaccia utente)
├─ Modbus RTU (RS485) ← ESP32 Controller
└─ App ViCare (remoto via Vitoconnect)
```

## Manutenzione

### Mensile
- ✅ Verificare letture Vitotronic 200
- ✅ Controllo assenza perdite

### Annuale
- 🔄 Spurgo aria circuiti (se pressione anomala)
- 🔄 Verifica isolamento tubi
- 🔄 Controllo integrità vaso espansione
- 🔄 Pulizia sensori temperatura

### Se Allarme Vitotronic 200
- Leggere codice errore su display
- Verificare comunicazione Modbus
- Controllare sensori temperatura
- Contattare tecnico se persistente

## Integrazione ESP32 via Modbus RTU

### Cosa è un ESP32?

L'ESP32 è un **microcontroller Arduino che TU aggiungi opzionalmente** per:
- Leggere dati dal Vitotronic 200 (temperature, stato pompa, errori)
- Inviare comandi (setpoint temperatura, modalità riscaldamento/raffreddamento)
- Acquisire sensori aggiuntivi (umidità, qualità aria, ecc)
- Integrare con domotica (Home Assistant, Node-RED, MQTT)
- Creare automazioni custom
- Registrare dati storici

**Il Vitotronic 200 funziona perfettamente ANCHE SENZA ESP32** - l'ESP32 è per chi vuole automazione avanzata/monitoraggio.

### Come Collegare ESP32 al Vitotronic 200

**Connessione fisica**: Porta RS485 del Vitotronic 200 → Modulo RS485 USB → ESP32

```
┌──────────────────────────────────┐
│    VITOTRONIC 200                │
│    (dentro unità interna)        │
│                                  │
│    Porta RJ45 (RS485)            │
│    ├─ Pin 1: A+ (Dati+)          │
│    ├─ Pin 2: B− (Dati−)          │
│    ├─ Pin 3: GND                 │
│    └─ Pin 8: +5V (opzionale)     │
└──────────────┬───────────────────┘
               │
        Cavo RS485 (2 fili)
        Lunghezza max: ~30 metri
               │
    ┌──────────▼──────────┐
    │  Modulo RS485 MAX485 │
    │  (convertitore)      │
    │  A ← Pin 1           │
    │  B ← Pin 2           │
    │  GND ← Pin 3         │
    │  DI → ESP32 GPIO RX  │
    │  RO ← ESP32 GPIO TX  │
    │  DE/RE → ESP32 GPIO  │
    └──────────┬───────────┘
               │
    ┌──────────▼──────────┐
    │     ESP32           │
    │   (UART1 o 2)       │
    └─────────────────────┘
```

### Protocollo Modbus RTU

**Baudrate**: 9600 baud
**Indirizzo slave Vitotronic 200**: 1 (fisso)
**Formato frame**:
```
[INDIRIZZO] [FUNZIONE] [REGISTRO] [LUNGHEZZA] [CRC_LO] [CRC_HI]
```

**Funzioni Modbus supportate**:
- Funzione 3: Lettura Holding Registers (letture di stato)
- Funzione 6: Scrittura singolo Register (comandi semplici)
- Funzione 16: Scrittura multipli Registers (comandi complessi)

### Registri Modbus Vitotronic 200 Principali

```
=== LETTURE (Holding Registers) ===

REG 0: T_mandata (°C × 10)
       Temperatura acqua uscita Vitocal
       Valore: 550 = 55,0°C

REG 1: T_ritorno (°C × 10)
       Temperatura acqua rientro circuito

REG 2: T_boiler (°C × 10)
       Temperatura accumulatore termico (SPCF 100L)

REG 3: T_esterna (°C × 10)
       Temperatura aria esterna (da sensore)

REG 4: T_ACS (°C × 10)
       Temperatura acqua calda sanitaria

REG 5: Potenza compressore (%)
       0 = spento, 100 = massimo

REG 6: Stato Modalità
       0 = Spento
       1 = Riscaldamento (Pompa Giorno)
       2 = Riscaldamento (Pompa Notte)
       3 = Raffrescamento
       4 = ACS
       5 = Protezione Antigelo

REG 7: Pressione sistema (bar × 10)
       Valore: 15 = 1,5 bar

REG 8: Pompa solare attiva? (0=NO, 1=SI)

REG 100: Errore/Allarme (codice)
         0 = OK
         1 = Pressione bassa
         2 = T_mandata troppo alta
         3 = Flussostato assente
         ...

=== COMANDI (Scrittibili) ===

REG 101: Setpoint temperatura riscaldamento (°C × 10)
         Range: 200-550 (20-55°C)
         Comando: Scrivi 220 per 22°C

REG 102: Setpoint temperatura raffrescamento (°C × 10)
         Range: 150-280 (15-28°C)

REG 103: Setpoint temperatura ACS (°C × 10)
         Range: 300-600 (30-60°C)
         Default: 500 (50°C)

REG 104: Velocità ventola fan-coil (#1)
         0 = Auto
         1 = Minima
         2 = Notte
         3 = Massima

REG 105: Velocità ventola fan-coil (#2)
         (stesso formato)

REG 106: Modalità operativa (Comando)
         1 = Riscaldamento
         2 = Raffrescamento
         3 = ACS
         0 = Standby
```

### Esempio Lettura con ESP32 (Arduino IDE)

```cpp
#include <ModbusMaster.h>  // Libreria ModbusMaster

ModbusMaster node;

void setup() {
  Serial.begin(9600);
  node.begin(1, Serial1);  // Indirizzo 1, UART1
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

void loop() {
  // Lettura Registro 0 (T_mandata)
  uint8_t result = node.readHoldingRegisters(0, 1);
  
  if (result == node.ku8MBSuccess) {
    int16_t t_mandata_raw = node.getResponseBuffer(0);
    float t_mandata = t_mandata_raw / 10.0;
    
    Serial.print("Temperatura mandata: ");
    Serial.print(t_mandata);
    Serial.println(" °C");
  }
  
  delay(2000);  // Leggi ogni 2 secondi
}

void preTransmission() {
  digitalWrite(DE_RE_PIN, HIGH);  // Abilita trasmissione
}

void postTransmission() {
  digitalWrite(DE_RE_PIN, LOW);   // Disabilita (ricezione)
}
```

### Esempio Comando con ESP32

```cpp
// Imposta setpoint riscaldamento a 22°C
uint16_t aData[1] = { 220 };  // 220 = 22,0°C (×10)
uint8_t result = node.writeMultipleRegisters(101, 1, aData);

if (result == node.ku8MBSuccess) {
  Serial.println("Setpoint 22°C impostato!");
} else {
  Serial.println("Errore Modbus!");
}
```

### Monitoraggio Real-Time (Home Assistant Example)

```yaml
modbus:
  - name: Viessmann
    type: tcp
    host: 192.168.1.100  # Indirizzo ESP32
    port: 502
    sensors:
      - name: "Temperatura Mandata"
        unit_of_measurement: °C
        register: 0
        scale: 0.1
      - name: "Temperatura Boiler"
        unit_of_measurement: °C
        register: 2
        scale: 0.1
      - name: "Potenza Compressore"
        unit_of_measurement: "%"
        register: 5
```

## Regolazione Vitotronic 200 - Comandi Modbus Veloci

```
LETTURE FREQUENTI:
REG 0  = T_mandata          REG 5 = Potenza compressore
REG 2  = T_boiler           REG 6 = Stato modalità
REG 4  = T_ACS              REG 100 = Allarmi

COMANDI COMUNI:
REG 101 = Setpoint riscaldamento (es. 220 = 22°C)
REG 102 = Setpoint raffrescamento (es. 260 = 26°C)
REG 104 = Velocità ventola fan-coil #1
REG 106 = Modalità (1=riscaldamento, 2=raffrescamento)
```

---

*Fonte: Datasheet Viessmann Vitocal 100-S Rev. 06/2018*
*Aggiornamento: Aprile 2026*
