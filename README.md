# Viessmann Waveshare S3 — ESP32-S3-Touch-LCD-4.3B Controller

Controllo dei ventilconvettori **Viessmann Energycal Slim W** via **Modbus ASCII RS485** con display touch LVGL 800x480 e WiFi.

## Status

✅ **FUNZIONANTE** — Il firmware manda gli stessi comandi del master originale e controlla perfettamente i ventilconvettori.

**Ultimi test (Aprile 2026):**
- ✅ ACCENDI: ventilconvettori si accendono
- ✅ SPEGNI: ventilconvettori si spengono
- ✅ Keep-alive automatico ogni 68 secondi
- ✅ Registri Modbus sincronizzati con master originale

## Hardware Setup

### Waveshare ESP32-S3-Touch-LCD-4.3B

La board ha **2 levette selettore** sulla parte posteriore:

1. **CAN/RS485 Switch**
   - Posizione **RS485** ← **DEVE ESSERE QUI** per Modbus ASCII
   - Posizione CAN = per CAN bus (non usato qui)
   - **Funzione:** seleziona il protocollo di comunicazione con i ventilconvettori

2. **ON/KE Switch** 
   - Posizione **ON** ← **DEVE ESSERE QUI** per abilitare la resistenza di terminazione 120Ω
   - Posizione KE = disattiva resistenza (per bus molto corti)
   - **Funzione:** la resistenza 120Ω termina il bus RS485 e previene rimbalzi di segnale

⚠️ **Se le levette non sono impostate correttamente, il Modbus ASCII non funzionerà.**

### Connessione RS485

Connettore verde a 4 pin sulla board:
```
[A]  [B]  [GND] [+5V]
 |    |     |     |
 +----+-----+-----+--> Ventilconvettori (fili bianco/marrone)
```

- **A** = filo bianco (data line A)
- **B** = filo marrone (data line B)
- **GND** = massa comune
- **+5V** = alimentazione (opzionale, se i ventilconvettori la forniscono)

### Alimentazione Esterna (Consigliato)

L'S3 può funzionare con i 6V dal circuito dei ventilconvettori, ma è **fortemente consigliato** collegare un alimentatore esterno **5V-12V** per garantire:
- Trasmissione Modbus stabile
- Potenza sufficiente per il modulo RS485
- Evitare brownout e reset improvvisi

## Firmware

### Perché 3 Registri?

Il ventilconvettore Viessmann ha **3 parametri indipendenti** che devono essere controllati insieme:

1. **REG 101 — "Cosa fare?"**
   - Accendi/Spegni
   - Modalità (CALDO/FREDDO)
   - Velocità ventola (0-3)
   - Flag STANDBY (spegnimento)

2. **REG 102 — "A quale temperatura?"**
   - Temperatura desiderata in °C × 10
   - Es: `0x00CD` = 20.5°C, `0x00E1` = 22.5°C

3. **REG 103 — "In quale modalità?"**
   - Modo stagionale / manuale
   - Flag aggiuntivi di controllo

**Perché non bastano 1-2 registri?** Se dici solo "accendi" senza specificare temperatura e modalità, il ventilconvettore non sa come comportarsi. Devono sempre arrivare insieme: "Accendi in CALDO a 22°C in modalità stagionale" = tutti e 3 i registri contemporaneamente.

Il master originale fa così, e il nostro ESP fa lo stesso.

### Modalità di Funzionamento

**Broadcast (addr=0x00):**
- Invia comandi a **tutti i ventilconvettori contemporaneamente**
- Indirizzo Modbus: `0x00` (broadcast)
- Delay tra registri: 500ms (per evitare collisioni)

**Keep-alive periodico (68 secondi):**
- Ogni 68 secondi, il firmware rinvia automaticamente lo stato attuale (ON/OFF)
- Funziona **anche quando spento** — invia il comando di SPEGNIMENTO periodicamente
- Mantenimento dello stato, come il master originale

### Comandi Modbus ASCII — Valori Effettivi

Tre registri principali (sincronizzati con master originale):

| Registro | ON | OFF | Significato |
|----------|--------|---------|-------------|
| **101** | `0x4003` | `0x4083` | Config (FREDDO, FAN, STANDBY) |
| **102** | `0x32` | `0x32` | Temperatura 5.0°C (fisso) |
| **103** | `0xb9` | `0xb9` | Modo stagionale (fisso) |

### Bit Mapping (Registro 101)

```
bit14 = COLD (freddo)
bit13 = HEAT (caldo)  
bit7  = STANDBY/OFF (spegnimento)
bit1-0= FAN speed (00=off, 01=min, 10=auto, 11=max)
```

**Esempi:**
- `0x2003` = CALDO + FAN MAX (acceso)
- `0x2083` = CALDO + STANDBY (spento)
- `0x4003` = FREDDO + FAN MAX (acceso)
- `0x4083` = FREDDO + STANDBY (spento)

## Tabella Comandi — Codici Modbus

### Comandi inviati dal nostro ESP (Touch Display) — FULLY TESTED ✅

**ON/OFF:**

| Azione | REG 101 | REG 102 | REG 103 | Descrizione |
|--------|---------|---------|---------|-------------|
| **ACCENDI** | `0x4003` | Mantiene valore utente | `0xb9` | FREDDO MAX con temp impostata |
| **SPEGNI** | `0x4083` | `0x32` | `0xb9` | FREDDO + STANDBY, reset 5.0°C |
| **Keep-alive (68s)** | Stato attuale | Valore attuale | `0xb9` | Rinvia periodicamente |

**CALDO/FREDDO:**

| Modalità | REG 101 (FAN3) | bit14 | bit13 | Descrizione |
|----------|---------|-------|-------|-------------|
| **FREDDO** | `0x4003` | 1 | 0 | Modalità default |
| **CALDO** | `0x2003` | 0 | 1 | Alterna bit14↔bit13 |

**VENTOLA (FAN SPEED):**

| Velocità | Nome | REG 101 | bit1-0 | Descrizione |
|----------|------|---------|--------|-------------|
| **0** | OFF | `0x4000` | `00` | Ventola spenta |
| **1** | MIN | `0x4001` | `01` | Minima velocità |
| **2** | NIGHT/AUTO | `0x4002` | `10` | Modalità notturna |
| **3** | MAX | `0x4003` | `11` | Massima velocità |

**TEMPERATURA (TEMP +/-):**

| Azione | REG 102 | Temperatura | Step | Range |
|--------|---------|------------|------|-------|
| Minimo | `0x32` | 5.0°C | - | Anticongelamento |
| Incrementa (TEMP+) | +5 | +0.5°C | Ogni pressione | 5-40°C |
| Decrementa (TEMP-) | -5 | -0.5°C | Ogni pressione | 5-40°C |
| Massimo | `0x100` | 40.0°C | - | Limite superiore |
| Esempio | `0xCD` | 20.5°C | - | 205 ÷ 10 = 20.5 |

### Comandi del Master Originale (Catturati in reverse engineering)

**Comportamento:**
- Accende di default a **16.0°C** (salta da 5°C)
- Incrementa/decrementa di **0.5°C** tra 16-28°C
- Salta a **40°C** quando supera 28°C
- Spegne con reset a **5.0°C**

### Test Completati (Aprile 2026)

**Status: ✅ PRODUZIONE READY**

- ✅ ON/OFF — ventilconvettori rispondono perfettamente
- ✅ CALDO/FREDDO — alterna modalità correttamente
- ✅ FAN SPEED (0-3) — tutti i 4 livelli funzionano
- ✅ TEMPERATURA (5-40°C) — incrementi di 0.5°C
- ✅ Keep-alive ogni 68 secondi — mantiene stato
- ✅ Registri sincronizzati con master originale
- ✅ Display touch LVGL responsivo e funzionale
- ✅ Pagine web /sniffer e /test operazionali

## Controllo

### Pagine Web

- **`/`** — Status dashboard (stato attuale)
- **`/sniffer`** — Monitoraggio passivo frame Modbus in tempo reale
- **`/test`** — Test manuale registri (22 comandi predefiniti + custom)

### Web API (HTTP)

```bash
# Status
curl http://192.168.0.92/api/status

# Power (on/off)
curl "http://192.168.0.92/api/power?state=1"
curl "http://192.168.0.92/api/power?state=0"

# Temperature (5-35°C)
curl "http://192.168.0.92/api/temperature?value=22.5"

# Mode (heat/cool)
curl "http://192.168.0.92/api/mode?value=heat"
curl "http://192.168.0.92/api/mode?value=cool"

# Fan (0=off, 1=min, 2=auto, 3=max)
curl "http://192.168.0.92/api/fan?value=3"
```

### LVGL Touch Display

- Schermata principale con temperatura, pulsanti ON/OFF, modalità, fan
- Interfaccia reattiva aggiornata in tempo reale
- Touch responsivo a 60 FPS

### Seriale USB

Comandi testuali via seriale a 115200 baud:
```
ON              # Accendi
OFF             # Spegni
HEAT            # Modalità caldo
COOL            # Modalità freddo
T22.5           # Imposta temperatura 22.5°C
FAN3            # Imposta fan max (0-3)
SEND            # Invia registri manualmente
STATUS          # Mostra stato JSON
```

## Troubleshooting

### I ventilconvettori non rispondono

1. ✅ Verifica le levette sulla board:
   - CAN/RS485 = **RS485**
   - ON/KE = **ON**

2. ✅ Verifica i cavi RS485:
   - Filo bianco = A
   - Filo marrone = B
   - Massa comune

3. ✅ Aggiungi alimentazione esterna 5V-12V se manca

4. ✅ Controlla che il master originale non interferisca (collisioni Modbus)

### I LED rossi PWR/DONE sono troppo luminosi

Il firmware disabilita i LED via I2C (CH422G) al boot. Se rimangono accesi, copri con nastro opaco.

## Sviluppo

### Compilare il Firmware

```bash
pio run -e esp32s3-43b
```

### Flashing via USB-C

```bash
pio run -e esp32s3-43b -t upload --upload-port /dev/cu.usbmodem1101
```

### Sniffer Mode (Reverse Engineering)

Branch `sniffer-mode`: ascolta il master originale senza trasmettere.

```bash
git checkout sniffer-mode
pio run -e esp32s3-43b -t upload
```

## File Importanti

- `src/main.cpp` — firmware principale
- `src/lv_conf.h` — configurazione LVGL
- `platformio.ini` — configurazione build
- `tools/sniffer.py` — strumento interattivo per catturare frame Modbus

## Riferimenti

- [Waveshare ESP32-S3-Touch-LCD-4.3B](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3B)
- [Viessmann Energycal Slim W — Modbus ASCII](docs/viessmann-modbus.md)
- [LVGL Documentation](https://docs.lvgl.io/)

## Changelog Recente

### Aprile 2026
- ✅ **Reverse engineering completo** — Catturati e replicati comandi del master originale
- ✅ **Implementato keep-alive** — Invia stato ogni 68 secondi (identico al master)
- ✅ **Registri sincronizzati** — REG 101/102/103 identici al master per ON/OFF
- ✅ **Test hardware completati** — Ventilconvettori controllati perfettamente
- ✅ **Pagine web funzionanti** — /sniffer per reverse engineering, /test per debugging

---

**Autore:** Francesco Dal Savio | **Status:** ✅ Produzione | **Ultimo aggiornamento:** Aprile 2026
