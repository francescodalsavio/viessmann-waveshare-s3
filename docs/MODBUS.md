# Protocollo Modbus ASCII RS485 — Ventilconvettori Energycal Slim

## Introduzione

Il protocollo **Modbus ASCII RS485** è il linguaggio che i ventilconvettori Energycal Slim comprendono. Il nostro ESP32 comunica via Modbus ASCII per controllare i ventilconvettori.

**Caratteristiche**:
- **Protocollo**: Modbus ASCII (non binario)
- **Collegamento**: RS485 (master-slave)
- **Baud rate**: 9600 baud
- **Formato**: ASCII hex, non binario
- **Indirizzamento**: 0x00 (broadcast), 1-30 (singoli dispositivi)

---

## Tre Registri Principali

I ventilconvettori controllano **tre registri simultaneamente**:

### REG 101 — Configurazione e Controllo

**Cosa controlla**: Accensione/spegnimento, modalità caldo/freddo, velocità ventilatore, standby.

**Struttura binaria** (16 bit):

```
bit15  bit14  bit13  bit12  bit11  bit10  bit9  bit8
 ?    COLD   HEAT    ?      ?      ?      ?     ?

bit7   bit6   bit5   bit4   bit3   bit2   bit1  bit0
STBY   ?      ?      ?      ?      ?     FAN1  FAN0
```

#### Bit Mapping Dettagliato

| Bit | Nome | Funzione | Valore |
|-----|------|----------|--------|
| **14** | **COLD** | Modalità FREDDO (raffrescamento) | 1 = FREDDO, 0 = SPENTO |
| **13** | **HEAT** | Modalità CALDO (riscaldamento) | 1 = CALDO, 0 = SPENTO |
| **7** | **STBY** | STANDBY/OFF (spegnimento) | 1 = OFF, 0 = ON |
| **1-0** | **FAN** | Velocità ventilatore | 00=OFF, 01=MIN, 10=AUTO, 11=MAX |

#### Preset Combinazioni REG 101

| Comando | Valore | Binario | Descrizione |
|---------|--------|---------|-------------|
| **ON FREDDO MAX** | 0x4003 | 0100 0000 0000 0011 | bit14=1 (FREDDO), bit1-0=11 (FAN MAX) |
| **OFF FREDDO** | 0x4083 | 0100 0000 1000 0011 | bit14=1 (FREDDO), bit7=1 (STANDBY) |
| **ON CALDO MAX** | 0x2003 | 0010 0000 0000 0011 | bit13=1 (CALDO), bit1-0=11 (FAN MAX) |
| **OFF CALDO** | 0x2083 | 0010 0000 1000 0011 | bit13=1 (CALDO), bit7=1 (STANDBY) |
| **FAN OFF** | 0x4000 | 0100 0000 0000 0000 | bit14=1 (FREDDO), bit1-0=00 (FAN OFF) |
| **FAN MIN** | 0x4001 | 0100 0000 0000 0001 | bit14=1 (FREDDO), bit1-0=01 (FAN MIN) |
| **FAN AUTO** | 0x4002 | 0100 0000 0000 0010 | bit14=1 (FREDDO), bit1-0=10 (FAN AUTO) |
| **FAN MAX** | 0x4003 | 0100 0000 0000 0011 | bit14=1 (FREDDO), bit1-0=11 (FAN MAX) |

---

### REG 102 — Temperatura Setpoint

**Cosa controlla**: Temperatura desiderata in °C × 10.

**Formato**: Valore numerico (0x00 - 0xFF)

**Conversione**: Valore HEX ÷ 10 = Temperatura in °C

#### Esempi

| HEX | DECIMALE | TEMPERATURA | Uso |
|-----|----------|-------------|-----|
| 0x32 | 50 | 5.0°C | Anticongelamento (default spento) |
| 0x64 | 100 | 10.0°C | Freddo intenso |
| 0xa0 | 160 | 16.0°C | Temperatura minima confortevole |
| 0xbe | 190 | 19.0°C | Ambiente fresco |
| 0xcd | 205 | 20.5°C | Comfort standard |
| 0xe1 | 225 | 22.5°C | Comfort elevato |
| 0xff | 255 | 25.5°C | Massimo caldo |
| 0x100 | 256 | 25.6°C | Limite superiore (40°C esteso) |

**Range**: 5°C - 40°C (0x32 - 0x100)

**Incremento/Decremento**: +5 o -5 (= ±0.5°C)

---

### REG 103 — Modo Stagionale

**Cosa controlla**: Configurazione modalità di funzionamento stagionale.

**Valore fisso**: **0xb9** (rimane costante durante l'uso)

**Descrizione**: Configurazione interna modo stagionale/automatico. Non cambia con i comandi ON/OFF.

---

## Frame Modbus ASCII

### Struttura Frame

Un frame Modbus ASCII ha questa struttura:

```
: [ADDR] [FUNC] [DATA] [LRC] \r\n
```

| Parte | Lunghezza | Significato |
|-------|-----------|-------------|
| `:` | 1 byte | Inizio frame (ASCII ':') |
| `[ADDR]` | 2 char | Indirizzo slave (00=broadcast, 01-30=indirizzo) |
| `[FUNC]` | 2 char | Funzione Modbus (06=Write Single Register) |
| `[DATA]` | 8 char | Indirizzo registro + valore (4 byte hex) |
| `[LRC]` | 2 char | Checksum LRC (Longitudinal Redundancy Check) |
| `\r\n` | 2 byte | Fine frame (CR LF) |

### Esempio Reale

**Comando**: Accendi ventilconvettore in FREDDO MAX (REG 101 = 0x4003)

```
:00 06 0065 4003 LRC \r\n
```

Breakdown:
- `:` — Inizio frame
- `00` — Indirizzo broadcast (tutti i ventilconvettori)
- `06` — Funzione: Write Single Register
- `0065` — Indirizzo registro: 0x0065 (101 decimale)
- `4003` — Valore: 0x4003 (ON FREDDO MAX)
- `LRC` — Checksum (calcolato automaticamente)
- `\r\n` — Fine frame

---

## Calcolo LRC (Longitudinal Redundancy Check)

L'**LRC** è un checksum semplice che verifica l'integrità del frame.

### Algoritmo

1. Somma tutti i byte del frame (da ADDR a ultimo byte del valore) in hexadecimale
2. Prendi il complemento a due: `LRC = (-(sum)) & 0xFF`
3. Converti in ASCII hex

### Esempio

Per il frame `:00 06 0065 4003`:

```
Sum = 0x00 + 0x06 + 0x00 + 0x65 + 0x40 + 0x03 = 0xAE
LRC = (-0xAE) & 0xFF = 0x52
```

Frame completo: `:00 06 0065 4003 52\r\n`

---

## Sequenza di Invio Comandi (Broadcast)

Quando accendi/spegni tutti i ventilconvettori, il sistema invia **3 registri in sequenza**:

```
Tempo 0ms:    Invia REG 101 (config/on-off)
              ↓
              delay 500ms
              ↓
Tempo 500ms:  Invia REG 102 (temperatura)
              ↓
              delay 500ms
              ↓
Tempo 1000ms: Invia REG 103 (modo)
              ↓
              delay completato
```

**Perché 3 registri?**
Perché i ventilconvettori hanno 3 parametri indipendenti:
- **Cosa fare** (on/off, caldo/freddo, ventilatore)
- **A quale temperatura** (setpoint)
- **In quale modalità** (stagionale)

Devono arrivare insieme per funzionare correttamente.

---

## Indirizzi Modbus

### Broadcast vs Singolo

#### Broadcast (Indirizzo 0x00)
Invia comando a **TUTTI i ventilconvettori contemporaneamente**.

```
:00 06 0065 4003 LRC \r\n
   ↑
   Broadcast: tutti ascoltano e rispondono
```

**Usato per**: Accensione/spegnimento contemporaneo di tutti i terminali.

#### Indirizzo Singolo (0x01 - 0x1E)
Invia comando a **UN SOLO ventilconvettore**.

```
:01 06 0065 4003 LRC \r\n  ← Venticolo 1
:02 06 0065 4003 LRC \r\n  ← Venticolo 2
:03 06 0065 4003 LRC \r\n  ← Venticolo 3
```

**Usato per**: Controllo singolo di terminali specifici (futura feature).

---

## Keep-Alive Periodico

Il sistema invia automaticamente uno **keep-alive ogni 68 secondi** per mantenere lo stato:

```
Ogni 68 secondi:
  ├─ Invia REG 101 (stato attuale ON/OFF)
  ├─ Invia REG 102 (temperatura attuale)
  └─ Invia REG 103 (modo)
```

**Scopo**: Prevenire che i ventilconvettori si resettino o perdano la configurazione.

---

## Funzioni Modbus Supportate

### 0x06 — Write Single Register

Scrive un valore in un singolo registro.

```
Comando:  :ADDR 06 REG_HI REG_LO VAL_HI VAL_LO LRC \r\n
Risposta: :ADDR 06 REG_HI REG_LO VAL_HI VAL_LO LRC \r\n
          (Echo della richiesta)
```

**Esempio**:
```
Comando:  :00 06 0065 4003 LRC \r\n
Risposta: :00 06 0065 4003 LRC \r\n
```

### 0x03 — Read Holding Registers

Legge il valore di uno o più registri.

```
Comando:  :ADDR 03 REG_HI REG_LO COUNT_HI COUNT_LO LRC \r\n
Risposta: :ADDR 03 BYTE_COUNT VAL1_HI VAL1_LO ... LRC \r\n
```

**Uso**: Per leggere lo stato attuale di un ventilconvettore.

---

## Timing Critico

| Operazione | Durata | Note |
|-----------|--------|------|
| Trasmissione frame | ~30ms | Dipende dalla lunghezza |
| Delay tra registri | 500ms | **Critico**: <500ms può causare collisioni |
| Keep-alive interval | 68 secondi | Invia automaticamente stato |
| Timeout risposta | 1 secondo | Se no risposta entro 1s, timeout |

---

## Troubleshooting Modbus

### Ventilconvettore Non Risponde
✅ Verifica:
- Cablaggio RS485 (A, B, GND)
- Baud rate: 9600
- Indirizzo ventilconvettore corretto
- LRC checksum corretto
- Ventilconvettore alimentato

### Frame Corrotto
✅ Verifica:
- LRC calcolato correttamente
- Nessun delay troppo breve tra registri
- Cavi schermati e intrecciati
- Nessuna interferenza EMI vicino ai cavi

### Solo 1-2 Ventilconvettori Rispondono
✅ Verifica:
- Indirizzo Modbus configurato correttamente (01, 02, 03...)
- Terminazione 120Ω ai capi del bus
- Alimentazione sufficiente
- Distanza cavi < 50m

---

## Conversione Rapida

### REG 101 - Combinazioni Comuni

**Freddo**:
- `0x4000` = Freddo (FAN OFF)
- `0x4001` = Freddo (FAN MIN)
- `0x4002` = Freddo (FAN AUTO)
- `0x4003` = Freddo (FAN MAX) ← **DEFAULT ON**

**Caldo**:
- `0x2000` = Caldo (FAN OFF)
- `0x2001` = Caldo (FAN MIN)
- `0x2002` = Caldo (FAN AUTO)
- `0x2003` = Caldo (FAN MAX)

**Spegnimento**:
- `0x4083` = Freddo + STANDBY ← **DEFAULT OFF**
- `0x2083` = Caldo + STANDBY

### REG 102 - Temperature Veloci

```
16°C  → 0xa0 (160 decimale)
18°C  → 0xb4 (180 decimale)
20°C  → 0xc8 (200 decimale)
22°C  → 0xdc (220 decimale)
24°C  → 0xf0 (240 decimale)
```

---

## Test Frame Modbus

### Frame di Test — Accensione FREDDO MAX

```
:00 06 0065 4003 FC \r\n
```

**Decodifica**:
- Broadcast (0x00)
- Write Single Register (0x06)
- Registro 0x0065 (101)
- Valore 0x4003 (ON FREDDO MAX)
- LRC: 0xFC

### Frame di Test — Impostazione Temperatura 22°C

```
:00 06 0066 00DC FB \r\n
```

**Decodifica**:
- Broadcast (0x00)
- Write Single Register (0x06)
- Registro 0x0066 (102)
- Valore 0x00DC (22°C)
- LRC: 0xFB

---

## Riferimenti

- **Standard**: Modbus Serial Line (IEC 61131-3)
- **Documentazione ufficiale**: http://www.modbus.org
- **Specifiche Viessmann**: Vedere datasheet Energycal Slim
- **Baud rate standard**: 9600, 8N1 (8 data bits, no parity, 1 stop bit)
