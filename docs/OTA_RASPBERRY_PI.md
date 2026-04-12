# OTA (Over-The-Air) Updates dal Raspberry Pi Molinella

**Guida per aggiornare l'ESP32 wireless dal Raspberry Pi senza USB cable.**

---

## Prerequisiti

- Raspberry Pi connesso allo stesso network WiFi di Casa Molinella
- Python 3.7+ installato
- Internet connection (per clonare il repo e installare PlatformIO)
- ESP32 online a `192.168.0.89` (o hostname `viessmann-controller.local`)

---

## 1️⃣ Installazione PlatformIO su Raspberry Pi

Esegui UNA SOLA VOLTA:

```bash
# Installa PlatformIO via pip
pip3 install platformio

# Verifica l'installazione
platformio --version
```

Se fallisce, prova:

```bash
python3 -m pip install platformio
```

---

## 2️⃣ Clona il Progetto sul Pi

```bash
# Vai nella home directory
cd ~

# Clona il repository
git clone https://github.com/francescodalsavio/viessmann-waveshare-s3.git

# Entra nella cartella
cd viessmann-waveshare-s3
```

---

## 3️⃣ Fare OTA Upload da Raspberry Pi

### Opzione A: Via IP Address (Raccomandato se la rete cambia spesso)

```bash
cd ~/viessmann-waveshare-s3

python3 -m platformio run -e esp32s3-43b -t upload --upload-port=192.168.0.89
```

### Opzione B: Via mDNS Hostname (Più elegante)

```bash
cd ~/viessmann-waveshare-s3

python3 -m platformio run -e esp32s3-43b -t upload --upload-port=viessmann-controller.local
```

**⭐ Preferire l'Opzione B** — non devi ricordarti l'IP!

---

## 4️⃣ Output Atteso

Durante l'upload vedrai:

```
Platform Manager: Installing espressif32 @ 6.4.0
...
Compiling .pio/build/esp32s3-43b/src/main.cpp.o
...
Uploading .pio/build/esp32s3-43b/firmware.bin
Uploading: [==============================                    ] 50%
Uploading: [========================================          ] 100%
Upload complete!
```

Poi l'**ESP32 si riavvia automaticamente** (~10 secondi).

---

## 5️⃣ Verifica che Funziona

### Su Raspberry Pi: Monitora il MQTT

```bash
mosquitto_sub -h localhost -t "viessmann/status"
```

Dovresti vedere i dati in tempo reale:

```json
{"temp": 22.5, "hvac": "heat", "fan": "high", "humidity": 45}
```

Se vedi dati, **OTA è riuscito!** ✅

### Su iPhone: Apri l'app Casa

- Vai su Viessmann Climate
- Cambia la velocità ventola
- Dovrebbe aggiornare in tempo reale

---

## 🔄 Workflow Veloce per Sviluppo

```bash
# 1. Modifica il codice sul Mac/PC
nano ~/viessmann-waveshare-s3/src/main.cpp  # (o VSCode)

# 2. Commit e push su GitHub
git add .
git commit -m "Update: cambio fan modes"
git push

# 3. Sul Raspberry Pi: pull e upload
cd ~/viessmann-waveshare-s3
git pull origin main
python3 -m platformio run -e esp32s3-43b -t upload --upload-port=viessmann-controller.local

# 4. Attendi il riavvio (~10 sec)
# 5. Verifica su iPhone o MQTT
mosquitto_sub -h localhost -t "viessmann/status"
```

**Tempo totale**: ~1-2 minuti dall'idea al codice in produzione! 🚀

---

## 🛠️ Troubleshooting

### Errore: `command not found: platformio`

```bash
# Usa il path completo
python3 -m platformio run -e esp32s3-43b -t upload --upload-port=192.168.0.89
```

### Errore: `Cannot connect to 192.168.0.89`

```bash
# Verifica che l'ESP32 sia online
ping 192.168.0.89

# Se non risponde, controlla:
# 1. ESP32 è acceso?
# 2. Stesso network WiFi?
# 3. Firewall non blocca la porta OTA (3232)?
```

### Errore: `mDNS hostname not found`

```bash
# Fallback all'IP
python3 -m platformio run -e esp32s3-43b -t upload --upload-port=192.168.0.89

# Oppure verifica il nome hostname:
ping viessmann-controller.local
```

### Upload bloccato al 30%

```bash
# Aumenta il timeout
python3 -m platformio run -e esp32s3-43b -t upload --upload-port=192.168.0.89 --upload-speed=115200
```

### ESP32 non si riavvia dopo upload

```bash
# Riavvia manualmente l'ESP32:
# - Stacca e attacca l'alimentazione, oppure
# - Premi il pulsante RESET fisico sul dispositivo
```

---

## 📋 Comandi Utili

| Comando | Funzione |
|---------|----------|
| `ping 192.168.0.89` | Verifica se ESP32 è online |
| `mosquitto_sub -h localhost -t "viessmann/#"` | Monitora tutti i topic MQTT |
| `git pull && pio run -e esp32s3-43b -t upload --upload-port=viessmann-controller.local` | Pull + Upload una riga (scorciatoia) |
| `platformio --version` | Verifica versione PlatformIO |
| `python3 -m platformio update` | Aggiorna PlatformIO |

---

## 🎯 Best Practices

✅ **Usa hostname `viessmann-controller.local`** — non dipende dall'IP

✅ **Monitora MQTT mentre fai modifiche** — vedi subito se funziona

✅ **Crea alias nel `.bashrc`** del Pi:

```bash
echo 'alias ota="cd ~/viessmann-waveshare-s3 && python3 -m platformio run -e esp32s3-43b -t upload --upload-port=viessmann-controller.local"' >> ~/.bashrc
source ~/.bashrc

# Poi basta digitare:
ota
```

✅ **Tieni il repo sempre aggiornato**:

```bash
cd ~/viessmann-waveshare-s3
git pull origin main  # Prima di ogni upload
```

---

## 🚀 Dopo il Primo Upload

Una volta che OTA funziona, puoi:

1. **Modificare il codice** da Mac/PC
2. **Pushare su GitHub**
3. **Pull dal Pi** (`git pull`)
4. **Uploadare in 2 minuti** senza USB!

---

## 📞 Se Serve Aiuto

Controlla:

1. **MQTT topic** (mosquitto_sub) — sono i dati arrivati?
2. **Log seriale** — se hai accesso seriale via USB, verifica i messaggi
3. **Network** — ping 192.168.0.89 funziona?
4. **Firewall** — blocca la porta OTA?

---

*Guida OTA - Viessmann Casa Molinella*  
*Ultimo aggiornamento: Aprile 2026*
