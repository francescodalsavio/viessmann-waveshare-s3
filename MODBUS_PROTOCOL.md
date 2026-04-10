# Modbus Protocol Reference — Viessmann Energycal Slim W

Guida completa ai valori esadecimali e bit mapping per controllare i ventilconvettori via Modbus ASCII RS485.

---

## 📡 Registri Modbus (3 registri essenziali)

| Registro | Nome | Funzione | Tipo |
|----------|------|----------|------|
| **101** | Config | ON/OFF, modalità CALDO/FREDDO, velocità ventola, STANDBY | Bit-field |
| **102** | Temperatura | Setpoint temperatura (value × 0.1 = °C) | uint16 |
| **103** | Modo | Stagionale/Manuale (raramente cambia) | uint16 |

---

## 🔧 Registro 101 — CONFIG (ON/OFF, MODALITÀ, VENTOLA)

### Bit Mapping (16-bit, Big-Endian)

```
Bit 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
     ?  C  H  ?  ?  ?  ?  ?  S  ?  ?  ?  ?  ?  F  F

C   = bit14 = COLD (Freddo/Cooling) = 1 = attivo, 0 = inattivo
H   = bit13 = HEAT (Caldo/Heating)  = 1 = attivo, 0 = inattivo
S   = bit7  = STANDBY/OFF            = 1 = standby, 0 = on
F-F = bit1-0 = FAN speed            = 00/01/10/11 = off/min/auto/max
```

### Combinazioni Comuni

#### 🔥 CALDO (Heating)
```
bit14=0 (C=0)  bit13=1 (H=1)  bit7=0 (ON)  bit1-0=11 (FAN MAX)

Binary: 0010 0000 0000 0011
Hex:    0x2003
Decimal: 8195

Lettura: CALDO + FAN MAX + ON
```

#### ❄️ FREDDO (Cooling) — Default
```
bit14=1 (C=1)  bit13=0 (H=0)  bit7=0 (ON)  bit1-0=11 (FAN MAX)

Binary: 0100 0000 0000 0011
Hex:    0x4003
Decimal: 16387

Lettura: FREDDO + FAN MAX + ON
```

#### 🛑 STANDBY (Spento)
```
Aggiungere bit7=1 ai valori sopra:

FREDDO + STANDBY:
Binary: 0100 0000 1000 0011
Hex:    0x4083
Decimal: 16515

CALDO + STANDBY:
Binary: 0010 0000 1000 0011
Hex:    0x2083
Decimal: 8323
```

### Tabella Completa REG 101

#### FREDDO (bit14=1, bit13=0)

| Ventola | Hex | Binario | ON/Standby | Descrizione |
|---------|-----|---------|------------|-------------|
| OFF (00) | `0x4000` | 0100 0000 0000 0000 | ON | FREDDO, ventola OFF |
| MIN (01) | `0x4001` | 0100 0000 0000 0001 | ON | FREDDO, ventola MIN |
| AUTO (10) | `0x4002` | 0100 0000 0000 0010 | ON | FREDDO, ventola AUTO/NIGHT |
| MAX (11) | `0x4003` | 0100 0000 0000 0011 | ON | FREDDO, ventola MAX (default) |
| OFF (00) | `0x4080` | 0100 0000 1000 0000 | STANDBY | FREDDO, ventola OFF |
| MIN (01) | `0x4081` | 0100 0000 1000 0001 | STANDBY | FREDDO, ventola MIN |
| AUTO (10) | `0x4082` | 0100 0000 1000 0010 | STANDBY | FREDDO, ventola AUTO |
| MAX (11) | `0x4083` | 0100 0000 1000 0011 | STANDBY | FREDDO, ventola MAX |

#### CALDO (bit14=0, bit13=1)

| Ventola | Hex | Binario | ON/Standby | Descrizione |
|---------|-----|---------|------------|-------------|
| OFF (00) | `0x2000` | 0010 0000 0000 0000 | ON | CALDO, ventola OFF |
| MIN (01) | `0x2001` | 0010 0000 0000 0001 | ON | CALDO, ventola MIN |
| AUTO (10) | `0x2002` | 0010 0000 0000 0010 | ON | CALDO, ventola AUTO |
| MAX (11) | `0x2003` | 0010 0000 0000 0011 | ON | CALDO, ventola MAX |
| OFF (00) | `0x2080` | 0010 0000 1000 0000 | STANDBY | CALDO, ventola OFF |
| MIN (01) | `0x2081` | 0010 0000 1000 0001 | STANDBY | CALDO, ventola MIN |
| AUTO (10) | `0x2082` | 0010 0000 1000 0010 | STANDBY | CALDO, ventola AUTO |
| MAX (11) | `0x2083` | 0010 0000 1000 0011 | STANDBY | CALDO, ventola MAX |

---

## 🌡️ Registro 102 — TEMPERATURA

### Formula

```
Valore esadecimale × 0.1 = Temperatura in °C

Esempio:
0xCD = 205 (decimale) → 205 × 0.1 = 20.5°C
0xE1 = 225 (decimale) → 225 × 0.1 = 22.5°C
```

### Tabella Temperature (5-40°C)

| °C | Hex | Decimal | Binario | Note |
|----|-----|---------|---------|------|
| **5.0** | `0x32` | 50 | 0000 0000 0011 0010 | Min (anticongelamento) |
| **5.5** | `0x37` | 55 | 0000 0000 0011 0111 | |
| **6.0** | `0x3C` | 60 | 0000 0000 0011 1100 | |
| **10.0** | `0x64` | 100 | 0000 0000 0110 0100 | |
| **15.0** | `0x96` | 150 | 0000 0000 1001 0110 | |
| **16.0** | `0xA0` | 160 | 0000 0000 1010 0000 | Default master originale |
| **18.0** | `0xB4` | 180 | 0000 0000 1011 0100 | |
| **20.0** | `0xC8` | 200 | 0000 0000 1100 1000 | |
| **20.5** | `0xCD` | 205 | 0000 0000 1100 1101 | |
| **21.0** | `0xD2` | 210 | 0000 0000 1101 0010 | |
| **22.0** | `0xDC` | 220 | 0000 0000 1101 1100 | |
| **22.5** | `0xE1` | 225 | 0000 0000 1110 0001 | |
| **24.0** | `0xF0` | 240 | 0000 0000 1111 0000 | Massimo firmware nostro |
| **28.0** | `0x118` | 280 | 0000 0001 0001 1000 | Massimo master originale |
| **40.0** | `0x190` | 400 | 0000 0001 1001 0000 | Limite massimo assoluto |

### Calcolo da Temperatura a Hex

```cpp
// C++ — Calcola hex da gradi Celsius
float celsius = 22.5;
uint16_t regValue = (uint16_t)(celsius * 10);  // 225
// hex: 0xE1
```

---

## 🎛️ Registro 103 — MODO (Stagionale/Manuale)

Quasi mai cambia, valore fisso: `0xb9`

```
Valore:  0xb9 = 185 (decimale) = 1011 1001 (binario)
Lettura: Modo stagionale/automatico

(Non cambia nel nostro uso)
```

---

## 📋 Sequenza Comandi Tipici

### 🔴 ACCENDI (Power ON)

```
REG 101: 0x4003  (FREDDO + FAN MAX + ON)
REG 102: 0xCD    (20.5°C o temperatura desiderata)
REG 103: 0xb9    (Modo stagionale)

Delay: 20ms tra registri
Ripetizione: Ogni 68 secondi (keep-alive)
```

### 🔵 SPEGNI (Power OFF)

```
REG 101: 0x4083  (FREDDO + FAN MAX + STANDBY)
REG 102: 0x32    (Reset 5.0°C)
REG 103: 0xb9    (Modo stagionale)

Delay: 20ms tra registri
```

### 🔄 CAMBIA TEMPERATURA

```
REG 102: 0xCD → 0xE1  (20.5°C → 22.5°C)

(Solo questo registro cambia)
(REG 101 e 103 rimangono costanti)
```

### 🔀 CAMBIA MODALITÀ (CALDO/FREDDO)

```
Prima: 0x4003 (FREDDO)
Dopo:  0x2003 (CALDO)

(Alterna bit14 ↔ bit13)
```

### 🌀 CAMBIA VELOCITÀ VENTOLA

```
Attuali bit14-13: FREDDO (10)
Attuali bit7: ON (0)

OFF (00):   0x4000
MIN (01):   0x4001
AUTO (10):  0x4002
MAX (11):   0x4003  ← Default
```

---

## ⚡ Quick Reference — Comandi Frequenti

```cpp
// ACCENDI
REG101 = 0x4003;  // FREDDO + FAN MAX
REG102 = 0xCD;    // 20.5°C

// SPEGNI
REG101 = 0x4083;  // FREDDO + STANDBY
REG102 = 0x32;    // 5.0°C

// TEMPERATURA +0.5°C
REG102 += 5;      // (valore hex + 5 = +0.5°C)

// TEMPERATURA -0.5°C
REG102 -= 5;      // (valore hex - 5 = -0.5°C)

// MODALITÀ CALDO
REG101 = (REG101 & ~0x4000) | 0x2000;  // Freddo → Caldo

// MODALITÀ FREDDO
REG101 = (REG101 & ~0x2000) | 0x4000;  // Caldo → Freddo

// VENTOLA MAX
REG101 = (REG101 & ~0x03) | 0x03;  // bit1-0 = 11

// VENTOLA OFF
REG101 = REG101 & ~0x03;            // bit1-0 = 00
```

---

## 🔍 Come Testare Nuovi Valori

### Via Web API

```bash
# GET stato attuale
curl http://192.168.0.92/api/status

# Invia registro custom (es: REG101 = 0x2003)
curl "http://192.168.0.92/api/test?reg=101&value=8195"

# Invia tutti e 3 registri
curl "http://192.168.0.92/api/test?r101=16387&r102=205&r103=185"
```

### Via Seriale USB

```
STATUS                  # Mostra stato attuale
T22.5                   # Imposta temperatura 22.5°C
HEAT                    # Modalità caldo
COOL                    # Modalità freddo
FAN3                    # Ventola max
ON / OFF                # Accendi/Spegni
```

### Via Sniffer

```bash
git checkout sniffer-mode
pio run -e esp32s3-43b -t upload
# Poi visita http://192.168.0.92/sniffer per monitorare i comandi del master originale
```

---

## 📚 Risorse

- **README.md** — Overview progetto
- **CLEAN_ARCHITECTURE.md** — Architettura software
- **TESTING_GUIDE.md** — Unit testing
- **src/domain/use_cases_v2.h** — Logica dei comandi (SetTemperatureUseCase, etc)
- **src/data/viessmann_repository_impl.h** — Implementazione invio registri Modbus

---

**Ultimo aggiornamento:** Aprile 2026
