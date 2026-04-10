# Viessmann Waveshare S3 — ESP32-S3-Touch-LCD-4.3B Controller

Controllo dei ventilconvettori **Viessmann Energycal Slim W** via **Modbus ASCII RS485** con display touch LVGL 800x480 e WiFi.

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

**Default (Broadcast addr=0x00):**
- Invia comandi a **tutti i ventilconvettori contemporaneamente**
- Indirizzo Modbus: `0x00` (broadcast)
- Delay tra registri: 500ms (per evitare collisioni)

### Comandi Modbus ASCII

Tre registri principali:

| Registro | Valore | Significato |
|----------|--------|-------------|
| **101** | `0x2003` | Config (bit13=CALDO, bit14=FREDDO, bit7=STANDBY, bit0-1=FAN) |
| **102** | `0x00CD` | Temperatura (÷10 = °C, es. 0x00CD = 20.5°C) |
| **103** | `0x008A` | Seasonal mode |

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

### Comandi inviati dal nostro ESP (Touch Display)

| Azione | REG 101 | REG 102 | REG 103 | Descrizione |
|--------|---------|---------|---------|-------------|
| **ACCENDI CALDO** | `0x2003` | `0x00CD` | `0x008A` | bit13=HEAT, FAN MAX |
| **ACCENDI FREDDO** | `0x4003` | `0x00CD` | `0x008A` | bit14=COLD, FAN MAX |
| **SPEGNI** | `0x2083`/`0x4083` | (stesso) | (stesso) | bit7=STANDBY |
| **MODALITÀ CALDO** | `0x2003` | (stesso) | `0x008A` \| 0x02 | bit13 acceso |
| **MODALITÀ FREDDO** | `0x4003` | (stesso) | `0x008A` & ~0x02 | bit14 acceso |
| **VENTOLA 0 (OFF)** | `0x2000` | (stesso) | (stesso) | bit1-0 = 00 |
| **VENTOLA 1 (MIN)** | `0x2001` | (stesso) | (stesso) | bit1-0 = 01 |
| **VENTOLA 2 (AUTO)** | `0x2002` | (stesso) | (stesso) | bit1-0 = 10 |
| **VENTOLA 3 (MAX)** | `0x2003` | (stesso) | (stesso) | bit1-0 = 11 |
| **TEMP +0.5°C** | (stesso) | +50 | (stesso) | Es: 0x00CD → 0x00D2 |
| **TEMP -0.5°C** | (stesso) | -50 | (stesso) | Es: 0x00CD → 0x00C8 |

### Comandi del Master Originale (Catturati)

| Azione | REG 101 | REG 102 | REG 103 | Note |
|--------|---------|---------|---------|------|
| **ACCENDI (ON)** | `0x4003` | `0x32` | `0xb9` | FREDDO MAX a 5.0°C |
| **SPEGNI (OFF)** | `0x4083` | `0x32` | `0xb9` | FREDDO + STANDBY |
| **Keep-alive (periodico)** | `0x4003` | `0x32` | `0xb9` | Inviato ogni ~68 secondi |

### Reverse Engineering Notes

**Osservazioni importanti:**
- Master invia **FREDDO** (bit14 acceso) come modalità di default
- **ON vs OFF:** cambia solo REG 101:
  - ON: `0x4003` (FREDDO MAX, bit1-0=11)
  - OFF: `0x4083` (FREDDO STANDBY, bit7 acceso)
- **REG 102 e REG 103 rimangono costanti** a `0x32` (5.0°C) e `0xb9`
- **Keep-alive periodico:** il master rinvia lo stesso comando ogni ~68 secondi per mantenere lo stato

**Prossimi test:**
- Premi CALDO sul master — cambiano i dati?
- Premi FREDDO — cambiano?
- Premi FAN 0, 1, 2, 3 — cambiano?
- Premi TEMP + / TEMP - — cambia REG 102?

## Controllo

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

---

**Autore:** Francesco Dal Savio | **Status:** Beta | **Ultimo aggiornamento:** Aprile 2026
