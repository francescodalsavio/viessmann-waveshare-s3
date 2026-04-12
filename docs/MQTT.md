# 📡 Guida MQTT - Home Assistant Integration

## Panoramica

Integrazione MQTT tra ESP32 (controller Viessmann) e Home Assistant via Mosquitto broker sul Raspberry Pi.

```
ESP32 (192.168.0.89)
    │
    ├─ Pubblica stato ogni 10 secondi
    └─ Riceve comandi da HA
         │
         ▼
Mosquitto MQTT (192.168.0.86:1883)
         │
         ▼
Home Assistant (localhost:8123)
         │
         ├─ Dashboard
         ├─ Automazioni
         └─ Notifiche
```

---

## 🔧 Configurazione Broker MQTT

### Installazione Mosquitto (Raspberry Pi)

```bash
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients

# Avviare il servizio
sudo systemctl start mosquitto
sudo systemctl enable mosquitto

# Verificare che sia attivo
mosquitto -v
# Output: mosquitto version 2.0.x starting...
```

### Verifica Connessione

```bash
# Test dalla linea di comando
mosquitto_sub -h 192.168.0.86 -t "viessmann/#" -v

# In un altro terminale, pubblicare un test
mosquitto_pub -h 192.168.0.86 -t "viessmann/test" -m "hello"
```

---

## 📨 Topic MQTT

### Publish (ESP32 → HA)

| Topic | Tipo | Range | Descrizione |
|-------|------|-------|-------------|
| `viessmann/power` | String | "ON" / "OFF" | Stato alimentazione |
| `viessmann/temp` | Float | 5 - 35 | Temperatura setpoint (°C) |
| `viessmann/fan` | Integer | 0-3 | 0=Auto, 1=Min, 2=Night, 3=Max |
| `viessmann/mode` | String | "heat"/"cool"/"off" | Modalità riscaldamento/raffrescamento |
| `viessmann/status` | JSON | - | Stato completo (timestamp + tutti i dati) |

### Subscribe (HA → ESP32)

| Topic | Payload | Azione |
|-------|---------|--------|
| `viessmann/set/power` | "ON" / "OFF" | Accende/spegne pompa di calore |
| `viessmann/set/temp` | 18 - 28 | Imposta temperatura setpoint |
| `viessmann/set/fan` | 0, 1, 2, 3 | Cambia velocità ventola |
| `viessmann/set/mode` | "heat" / "cool" / "off" | Cambia modalità operativa |

---

## 📤 Payload JSON - Status Completo

Pubblicato ogni 10 secondi su `viessmann/status`:

```json
{
  "timestamp": "2026-04-12T14:32:15Z",
  "power": true,
  "temperature": 20.5,
  "fan_speed": 3,
  "mode": "heat",
  "modbus_status": "OK",
  "rssi": -65,
  "uptime_seconds": 3600,
  "boiler_temp": 48.2,
  "return_temp": 35.8,
  "pressure_bar": 2.4
}
```

---

## 🏠 Home Assistant - Configurazione

### 1. Installare Mosquitto Add-on

**Settings → Add-ons → Add-on Store → Mosquitto broker**

```yaml
# /config/addons/mosquitto/data/options.json
{
  "logins": [
    {
      "username": "viessmann",
      "password": "your_secure_password"
    }
  ],
  "anonymous": false,
  "customize": {
    "active": false
  }
}
```

### 2. Configurare MQTT Integration

**Settings → Devices & Services → Create Automation**

YAML config (`/config/configuration.yaml`):

```yaml
mqtt:
  broker: 192.168.0.86
  username: viessmann
  password: your_secure_password
  port: 1883
  protocol: 3.1.1

sensor:
  - platform: mqtt
    name: "Viessmann Temperature"
    state_topic: "viessmann/temp"
    unit_of_measurement: "°C"
    value_template: "{{ value }}"
    icon: mdi:thermometer

  - platform: mqtt
    name: "Viessmann Fan Speed"
    state_topic: "viessmann/fan"
    value_template: |
      {% if value == '0' %}Auto
      {% elif value == '1' %}Min
      {% elif value == '2' %}Night
      {% elif value == '3' %}Max
      {% endif %}

  - platform: mqtt
    name: "Viessmann Mode"
    state_topic: "viessmann/mode"

switch:
  - platform: mqtt
    name: "Viessmann Power"
    state_topic: "viessmann/power"
    command_topic: "viessmann/set/power"
    payload_on: "ON"
    payload_off: "OFF"

climate:
  - platform: mqtt
    name: "Viessmann Thermostat"
    current_temperature_topic: "viessmann/temp"
    temperature_state_topic: "viessmann/temp"
    temperature_command_topic: "viessmann/set/temp"
    modes:
      - heat
      - cool
      - "off"
    mode_state_topic: "viessmann/mode"
    mode_command_topic: "viessmann/set/mode"
    min_temp: 18
    max_temp: 28
    temp_step: 0.5
```

Riavviare Home Assistant dopo le modifiche.

---

## 🔄 Flusso Comandi Completo

### Esempio: Cambiare Temperatura da 20°C a 21°C

1. **Utente interagisce con HA Dashboard**
   - Clicca su "Viessmann Thermostat" 
   - Regola slider a 21°C
   - Preme OK

2. **Home Assistant pubblica MQTT**
   ```
   Topic: viessmann/set/temp
   Payload: 21
   ```

3. **ESP32 riceve comando**
   - `mqttCallback()` legge il valore
   - Converte: `21 → 0x00D5` (210 in esadecimale)
   - Aggiorna `regConfig` (REG 102)
   - Chiama `sendAllRegisters()` con retry logic (default: 6 tentativi × 100ms)

4. **Modbus RTU → Viessmann**
   - Invia comando via RS485 a 9600 baud
   - Pompa di calore riceve setpoint 21°C
   - Modifica comportamento

5. **ESP32 pubblica feedback**
   - Pubblica su `viessmann/temp`: `21`
   - Pubblica su `viessmann/status`: JSON aggiornato
   - Display locale si aggiorna (LVGL updateUI)

6. **Home Assistant riceve feedback**
   - Legge `viessmann/temp`
   - Dashboard mostra 21°C
   - Storico/grafici aggiornati

---

## 🛠️ Debugging MQTT

### Monitorare tutti i messaggi

```bash
# Su Raspberry Pi, in tempo reale
mosquitto_sub -h 192.168.0.86 -t "viessmann/#" -v

# Aspettare un comando da HA
# Dovresti vedere:
# viessmann/set/temp 21
# viessmann/temp 21
# viessmann/status {...}
```

### Inviare comandi di test

```bash
# Accendere pompa
mosquitto_pub -h 192.168.0.86 -t "viessmann/set/power" -m "ON"

# Impostare modalità freddo
mosquitto_pub -h 192.168.0.86 -t "viessmann/set/mode" -m "cool"

# Impostare temperatura
mosquitto_pub -h 192.168.0.86 -t "viessmann/set/temp" -m "24"

# Cambiare velocità ventola (Auto/Min/Night/Max)
mosquitto_pub -h 192.168.0.86 -t "viessmann/set/fan" -m "3"
```

### Verificare stato ESP32

```bash
# Log seriale su ESP32
screen /dev/ttyUSB0 115200

# Dovresti vedere:
# MQTT Connected to 192.168.0.86:1883
# Publishing: {"power": true, "temp": 20.5, ...}
# Received: viessmann/set/temp = 21
# Modbus TX: REG 102 = 0x00D5
# Modbus RX OK
```

---

## ⚙️ Parametri Configurabili (via Settings Panel)

Gli utenti possono modificare da **Settings Panel (⚙️)** sul display touch:

| Parametro | Default | Range | Effetto |
|-----------|---------|-------|--------|
| `modbusRetries` | 6 | 1-10 | Tentativi trasmissione Modbus |
| `modbusRetryDelay` | 100 ms | 0-1000 (step 50) | Intervallo tra retry |
| `displaySleepTimeout` | 180 s | 30-600 (step 30) | Spegnimento display |

**Salvataggio**: NVS Flash → sopravvive a riavvii

---

## 🔐 Sicurezza MQTT

⚠️ **IMPORTANTE**: Mosquitto in locale su rete privata (192.168.0.x)

### Protezione Consigliata

1. **Username/Password**
   ```bash
   # Su Raspberry Pi
   sudo mosquitto_passwd -c /etc/mosquitto/passwd viessmann
   # Inserire password sicura
   ```

2. **Firewall**
   ```bash
   # Accettare MQTT solo da rete locale
   sudo ufw allow from 192.168.0.0/24 to any port 1883
   sudo ufw deny 1883
   ```

3. **Accesso Remoto via Cloudflare Tunnel**
   - Non esporre Mosquitto direttamente
   - Solo Home Assistant accede a Mosquitto localmente
   - Cloudflare Tunnel espone solo HA (porta 8123)
   - Vedi `CLOUDFLARE.md`

---

## 📊 Monitoraggio Avanzato

### Grafico Temperatura (HA)

```yaml
template:
  - sensor:
      - name: "Viessmann Setpoint"
        unique_id: "viessmann_setpoint"
        unit_of_measurement: "°C"
        state: "{{ states('sensor.viessmann_temperature') }}"
        attributes:
          fan: "{{ states('sensor.viessmann_fan_speed') }}"
          mode: "{{ states('sensor.viessmann_mode') }}"
```

### Automazione - Riscaldamento Programmato

```yaml
automation:
  - alias: "Riscaldamento mattina"
    trigger:
      platform: time
      at: "06:00:00"
    action:
      service: mqtt.publish
      data:
        topic: "viessmann/set/power"
        payload: "ON"
      service: mqtt.publish
      data:
        topic: "viessmann/set/mode"
        payload: "heat"
      service: mqtt.publish
      data:
        topic: "viessmann/set/temp"
        payload: "21"

  - alias: "Spegnimento notte"
    trigger:
      platform: time
      at: "22:00:00"
    action:
      service: mqtt.publish
      data:
        topic: "viessmann/set/power"
        payload: "OFF"
```

---

## 📞 Supporto

- **Mosquitto Docs**: https://mosquitto.org/documentation/
- **Home Assistant MQTT**: https://www.home-assistant.io/integrations/mqtt/
- **PubSubClient Arduino**: https://pubsubclient.knolleary.net/

---

*Documento aggiornato: Aprile 2026*
*Sistema Viessmann - Francesco Dal Savio*
