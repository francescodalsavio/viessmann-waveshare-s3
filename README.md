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
