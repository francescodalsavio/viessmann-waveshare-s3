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

## Controllo Temperatura Ambiente - Funzionamento Interno

### Cosa Fai Quando Imposti "22°C" sul Display?

Quando tu usi il Vitotronic 200 per impostare la temperatura ambiente a 22°C, **NON stai impostando la temperatura dell'acqua**, stai impostando il **SETPOINT (target) della stanza**.

```
TU → Display Vitotronic 200 → Premi [+] fino a 22°C
       ↓
VITOTRONIC MEMORIZZA: "Setpoint ambiente = 22°C"
       ↓
VITOTRONIC INIZIA LOOP CONTINUO:
  1. Leggi T_ambiente attuale (da sensore)
  2. Confronta con setpoint (22°C)
  3. Calcola errore di temperatura
  4. Modula pompa di calore per correggere
  5. Torna al passo 1 (ogni ~10 secondi)
```

### Sistema PID (Proportional-Integral-Derivative)

Il Vitotronic 200 usa un **algoritmo PID** di controllo automatico:

```
CICLO DI CONTROLLO (ogni ~10 secondi):

┌─────────────────────────────────────────┐
│ 1. LETTURA SENSORE                      │
│    T_ambiente attuale = 21,5°C          │
├─────────────────────────────────────────┤
│ 2. CALCOLO ERRORE                       │
│    Errore = Setpoint − T_attuale        │
│    Errore = 22°C − 21,5°C = +0,5°C      │
│    (+positivo = troppo freddo)          │
├─────────────────────────────────────────┤
│ 3. AZIONE PROPORZIONALE                 │
│    Errore +0,5°C → Aumenta compressore  │
│    di ~5-10% gradualmente               │
├─────────────────────────────────────────┤
│ 4. MODULAZIONE INVERTER                 │
│    Compressore: 40% → 45% → 50%...      │
│    (accelerazione graduale, NON on/off)  │
├─────────────────────────────────────────┤
│ 5. CONTROLLO VALVOLE & POMPE            │
│    Apri riscaldamento, pompa 80%,       │
│    fan-coil velocità media              │
├─────────────────────────────────────────┤
│ 6. RIPETI LOOP                          │
│    Ritorna al passo 1                   │
└─────────────────────────────────────────┘
```

### Esempio Reale: Scenario Mattina Fredda

**Situazione**: Ore 9:00 AM, casa a 18°C, tu imposti setpoint 22°C

```
TEMPO: 9:00 AM
─────────────────────────────────────────
Tu imposti 22°C sul display Vitotronic
       ↓
Lettura: T_ambiente = 18°C (sensore)
Errore = 22°C − 18°C = +4°C (FREDDO!)
       ↓
AZIONE MASSIMA (controllo aggressivo):
├─ Compressore pompa: 100% potenza
├─ Valvola riscaldamento: APERTA (55°C)
├─ Pompa circolazione: 100%
├─ Fan-coil #1: Velocità 3 (massima)
└─ Fan-coil #2: Velocità 3 (massima)

═══════════════════════════════════════════

TEMPO: 9:05 AM (5 minuti dopo)
─────────────────────────────────────────
Lettura: T_ambiente = 20°C (sale lentamente)
Errore = 22°C − 20°C = +2°C (ancora freddo)
       ↓
AZIONE MEDIA (controllo moderato):
├─ Compressore pompa: 70% potenza
├─ Pompa circolazione: 70%
├─ Fan-coil #1: Velocità 2 (media)
└─ Fan-coil #2: Velocità 2 (media)

═══════════════════════════════════════════

TEMPO: 9:15 AM (15 minuti dopo)
─────────────────────────────────────────
Lettura: T_ambiente = 22°C (raggiunto target!)
Errore = 22°C − 22°C = 0°C (PERFETTO!)
       ↓
AZIONE STANDBY (controllo delicato):
├─ Compressore pompa: 30% potenza (mantenimento)
├─ Pompa circolazione: 50% soft
├─ Fan-coil #1: Velocità 1 (minima, silenzioso)
└─ Fan-coil #2: Velocità 1 (minima, silenzioso)

═══════════════════════════════════════════

TEMPO: 9:20 AM (qualcuno apre finestra, temp cala)
─────────────────────────────────────────
Lettura: T_ambiente = 21,8°C (scende!)
Errore = 22°C − 21,8°C = +0,2°C (appena freddo)
       ↓
AZIONE MINORE (micro-correzione):
├─ Compressore pompa: 40% potenza
├─ Fan-coil: Velocità 1 (soft)
└─ Sistema mantiene 22°C senza sprechi
```

### Vantaggi del Controllo PID

✅ **Gradualità**: Modulazione continua, non "acceso/spento" (più comfort, meno rumore)
✅ **Efficienza**: Modula potenza al minimo necessario per mantenere target
✅ **Stabilità**: Mantiene temperatura stabile ±0,5°C
✅ **Protezione**: Non accelera pompa inutilmente (durata compressore)
✅ **Anticipazione**: Intelligenza integrale riduce oscillazioni

### Dove Legge la Temperatura Ambiente?

Il Vitotronic 200 può leggere la temperatura da **3 fonti**:

```
1️⃣ SENSORE INTERNO (dentro Vitotronic 200)
   ├─ Se il Vitotronic è montato in salotto
   └─ Misura temperatura della stanza locale
   
2️⃣ SONDA REMOTA WIRELESS (Vitoconnect)
   ├─ Sensore separato in camera/salotto
   ├─ Comunica via WiFi con Vitotronic
   └─ Più accurato (misura dove vivi)

3️⃣ ALGORITMO DINAMICO (default)
   ├─ Se nessun sensore esterno disponibile
   ├─ Usa T_mandata e T_ritorno del circuito
   └─ Stima approssimativa (meno precisa)
```

### Setpoint Giornaliero vs Notturno

Il Vitotronic 200 **supporta programmazione oraria** per risparmiare energia:

```
PROGRAMMAZIONE TIPICA:

Ore 06:00-09:00 (mattina)  → Setpoint 22°C (sveglia)
Ore 09:00-17:00 (giorno)   → Setpoint 20°C (fuori casa)
Ore 17:00-23:00 (sera)     → Setpoint 22°C (comfort)
Ore 23:00-06:00 (notte)    → Setpoint 18°C (sonno/risparmio)

Tu imposti questi orari e setpoint sul display una volta,
poi Vitotronic li segue automaticamente tutti i giorni!
```

### Comandi Modbus per Setpoint

Se usi un **ESP32 per controllare remoto**:

```cpp
// Imposta setpoint riscaldamento a 23°C
REG 101 = 230  // 230 × 0.1 = 23,0°C
Effetto: Vitotronic alza target da 22°C a 23°C

// Imposta setpoint raffrescamento a 26°C
REG 102 = 260  // 260 × 0.1 = 26,0°C
Effetto: Se modalità estate, fan-coil attiva a 26°C

// Leggi temperatura ambiente attuale
REG 0 = T_mandata (temperatura acqua uscita)
REG 2 = T_boiler (temperatura accumulo)
REG 3 = T_esterna (temperatura aria esterna)
```

---

**Riassunto**: Quando tu imposti 22°C, il Vitotronic 200 **automaticamente** accende/spegne/modula la pompa di calore per raggiungere e mantenere quella temperatura. È un ciclo continuo di misura-confronta-correggi, esattamente come un termostato intelligente.

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

### Registri Modbus Vitotronic 200 — Stato Documentazione

> ⚠️ **Viessmann NON pubblica i registri Modbus ufficialmente.**
> La tabella seguente viene da **reverse engineering** della community.
> Verificare sul campo — potrebbero variare tra modelli.

**Protocollo confermato da community:**
- Baudrate: **9600**
- Parity: **Even (E)**
- Slave address: **1**

**Registri noti (fonte: Epyon01P/viessmann su GitHub):**

| Registro | Tipo | Descrizione | Formula |
|---------|------|-------------|---------|
| **200** | Lettura | Modalità operativa | valore diretto |
| **400** | Lettura | Temperatura ritorno | ÷ 10 = °C |
| **401** | Lettura | Temperatura mandata | ÷ 10 = °C |
| **405** | Lettura | Temperatura ACS | ÷ 10 = °C |
| **428** | Lettura | Temperatura esterna | ÷ 10 = °C |
| **1002** | Scrittura | Setpoint riscaldamento | ÷ 10 = °C |
| **1003** | Scrittura | Setpoint ACS | ÷ 10 = °C |

> 📖 Per guida completa al test e altre risorse community:
> [COLLEGAMENTO-RS485-TEST.md](../COLLEGAMENTO-RS485-TEST.md)

### Esempio Lettura con ESP32 (Arduino IDE)

> ⚠️ Parity **SERIAL_8E1** (Even) — non SERIAL_8N1!

```cpp
#include <ModbusMaster.h>  // Libreria ModbusMaster

ModbusMaster node;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8E1);  // Even parity obbligatorio!
  node.begin(1, Serial1);           // Indirizzo 1, UART1
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

## Test Rapido da Mac con FT232RNL

Prima di usare un ESP32 permanente, puoi **testare la comunicazione Modbus RTU** dalla Vitocal direttamente dal Mac usando il convertitore **FT232RNL** (USB→RS485).

### Problema: connettori diversi

Il FT232RNL ha **morsetti a vite** (A+, B-, GND), la Vitocal ha una **porta RJ45**.  
Servono solo **€1-2** per adattarli.

### Opzione 1 — Cavo Ethernet Tagliato (€1-2) ✅ Consigliata

Prendi un qualsiasi **cavo ethernet patch** (CAT5/CAT6, 50cm) e taglia un'estremità:

```
[RJ45] ──────── fili scoperti
   ↑                  ↑
Vitocal           Morsetti FT232RNL
```

Collegamento fili (standard EIA-568B):

| Filo | Colore | Connetti a |
|------|--------|-----------|
| Pin 1 | Arancio/Bianco | **A+** sul FT232RNL |
| Pin 2 | Arancio | **B−** sul FT232RNL |
| Pin 3 | Verde/Bianco | **GND** sul FT232RNL |

- Costo: ~€1-2
- Probabilmente **già ce l'hai in casa**

### Opzione 2 — RJ45 Breakout Board (€5-8)

Adattatore professionale con **connettore RJ45 femmina → morsetti a vite** per tutti gli 8 pin:

```
[RJ45 femmina] → [morsetti a vite pin 1-8]
```

Cerca su Amazon: `"RJ45 breakout board"` o `"RJ45 terminal block adapter"`

### Schema Completo Collegamento Test

```
Mac (USB)
    ↓
FT232RNL (switch su RS485)
    ↓ morsetti A+ / B- / GND
Cavo ethernet tagliato
    ↓ RJ45
Vitotronic 200 (porta RS485 sul lato)
    ↓
Vitocal 100-S/111-S
```

### Test da Mac (Terminal)

```bash
# 1. Verifica porta
ls /dev/tty.* | grep usb
# → /dev/tty.usbserial-XXXXX

# 2. Installa pymodbus
pip install pymodbus

# 3. Test lettura temperatura mandata (REG 0)
python3 -c "
from pymodbus.client import ModbusSerialClient
c = ModbusSerialClient('serial', port='/dev/tty.usbserial-XXXXX', baudrate=9600)
c.connect()
r = c.read_holding_registers(0, 1, slave=1)
print(f'T_mandata: {r.registers[0]/10}°C')
c.close()
"
```

> 💡 **Vedi anche**: [COLLEGAMENTO-RS485-TEST.md](../COLLEGAMENTO-RS485-TEST.md) per guida completa con tutti i test.

---

*Fonte: Datasheet Viessmann Vitocal 100-S Rev. 06/2018*
*Aggiornamento: Aprile 2026*
