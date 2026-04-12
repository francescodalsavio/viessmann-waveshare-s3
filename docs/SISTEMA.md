# 🏠 Sistema Viessmann Completo - Documentazione Tecnica

**Casa Molinella - Impianto Intelligente**
*Controllore: Francesco Dal Savio*
*Ultimo aggiornamento: Aprile 2026*

---

## 📋 Sommario Componenti

| Componente | Modello | Spec Principale | Foto |
|-----------|---------|-----------------|------|
| **Scaldacqua (ACS)** | Viessmann Vitocal 060-A | Pompa calore ACS, 178-254L | [Bollitore interno](../images/hardware/scaldacqua/vitocal-060-a-bollitore.jpeg) |
| **Termostato Ambiente** | Cronotermostato touch + ESP32 | 230V AC, Modbus RTU | [thermostat/termostato-touch.jpeg](../images/hardware/thermostat/termostato-touch.jpeg) |
| **Boiler Accumulo** | Viessmann Vitocal 100-S | 97L, isolamento multistrato | [boiler/vitocal-100s.jpeg](../images/hardware/boiler/vitocal-100s.jpeg) · [Etichetta](../images/hardware/boiler/vitocal-100s-etichetta.jpeg) |
| **Pannello Solare** | Viessmann Solarcell SPCF 100L | 35W, 100L, collettore piano | [solar/spcf-100l.jpeg](../images/hardware/solar/spcf-100l.jpeg) |
| **Ventilconvettori** | 2× Fan-coil 4-tubi | Velocità variabile, 2 unità interne | [fan-coil/ventilconvettore-interno.jpeg](../images/hardware/fan-coil/ventilconvettore-interno.jpeg) |
| **Quadro Elettrico** | EATON | Interruttori C16, 4 circuiti pompa | [electrical/quadro-eaton.jpeg](../images/hardware/electrical/quadro-eaton.jpeg) |
| **Display Boiler** | Viessmann integrato | Lettura temperatura/pressione | [screenshots/viessmann-display.jpeg](../images/screenshots/viessmann-display.jpeg) |

---

## 🔄 Flusso Energetico Completo

```
┌─────────────────────────────────────────────────────────────┐
│                   FLUSSO INVERNO (RISCALDAMENTO)           │
└─────────────────────────────────────────────────────────────┘

ESTERNO:
┌─────────────────────┐
│ Aria Esterna (-5°C) │
└──────────┬──────────┘
           │ (scambio calore)
           ▼
    ┌──────────────────────────────────────┐
    │   POMPA CALORE VITOCAL 060           │
    │   (Aria-Acqua, 6kW riscaldamento)   │
    │                                      │
    │   • Compressore trifase 400V         │
    │   • Circuito frigorifero (R32)       │
    │   • Mandata: 70°C                    │
    │   • Ritorno: 45°C                    │
    │   • Modbus RTU comunicazione         │
    └──────────┬───────────────────────────┘
               │
               │ CIRCUITO IDRAULICO PRIMARIO
               ▼
    ┌──────────────────────────────────────┐
    │   BOILER ACCUMULO VITOCAL 100-S      │
    │   (97 litri, isolamento multistrato) │
    │                                      │
    │   • Mandata pompa calore: 70°C       │
    │   • Sonda temperatura alta           │
    │   • Sonda temperatura media (45°C)   │
    │   • Sonda temperatura bassa          │
    │   • Pressione nominale: 3 bar        │
    │   • Protezione termostato 60°C       │
    │                                      │
    │   ├─ ACS (Acqua Calda Sanitaria)    │
    │   │  ├─ Caldaia ausiliaria (se -°C) │
    │   │  └─ Pannello solare SPCF (pre-H)│
    │   │                                  │
    │   └─ Circuito Riscaldamento          │
    │      ├─ Valvola miscelatrice 3-vie  │
    │      ├─ Pompa di circolazione        │
    │      └─ Ventilconvettori (×2)        │
    │                                      │
    └──────────┬───────────────────────────┘
               │
               ├─────────────────────┬──────────────────┐
               ▼                     ▼                  ▼
        ┌────────────────┐   ┌────────────────┐  ┌────────────────┐
        │  FAN-COIL 1    │   │  FAN-COIL 2    │  │ RADIATORI (ACS)│
        │  SALOTTO       │   │  CAMERA        │  │ Punti caldi    │
        │  Mandata: 55°C │   │  Mandata: 55°C │  │                │
        │  Ritorno: 45°C │   │  Ritorno: 45°C │  │                │
        │  Velocità fan: │   │  Velocità fan: │  │                │
        │  0=Auto        │   │  0=Auto        │  │                │
        │  1=Min         │   │  1=Min         │  │                │
        │  2=Notte       │   │  2=Notte       │  │                │
        │  3=Max         │   │  3=Max         │  │                │
        └────────────────┘   └────────────────┘  └────────────────┘
               │                    │                  │
               │ ARIA CALDA         │ ARIA CALDA       │ ACQUA CALDA
               └────────┬───────────┴──────────────────┘
                        ▼
                    AMBIENTE
                   20-22°C
```

```
┌─────────────────────────────────────────────────────────────┐
│                   FLUSSO ESTATE (RAFFRESCAMENTO)            │
└─────────────────────────────────────────────────────────────┘

INTERNO:
┌──────────────────────────────┐
│ Ambiente Interno (28°C)      │
└──────────────┬───────────────┘
               │
               ▼
        ┌────────────────────────────────────────┐
        │  FAN-COIL 1 + FAN-COIL 2              │
        │  MODALITÀ RAFFRESCAMENTO              │
        │                                       │
        │  • Mandata fredda: 12-15°C           │
        │  • Ritorno: 20°C                     │
        │  • Condensa gestita                  │
        │  • Velocità ventola: AUTO/MIN/MAX    │
        └──────────┬───────────────────────────┘
                   │
                   │ CIRCUITO SECONDARIO
                   ▼
        ┌──────────────────────────────────────┐
        │   POMPA CALORE - MODALITÀ COOLING    │
        │   (Estrazione calore da ambienti)    │
        │                                      │
        │   • Ritorno freddo: 12°C             │
        │   • Scarico calore all'esterno: 35°C │
        │   • Compressore modulato             │
        └──────────┬───────────────────────────┘
                   │
                   ▼
            ARIA ESTERNA
           (dissipazione calore)
```

---

## 🔌 Circuito Idraulico Dettagliato

### Sezione Principale (Pompa ↔ Boiler)

```
┌─ POMPA CALORE VITOCAL 060
│  └─ MANDATA (70°C, 6kW)
│     │
│     ├─→ [Pressostato] → max 3.5 bar (alarm)
│     │
│     ├─→ [Sonda Mandata] → RTU REG 105
│     │
│     ▼
│  ┌──────────────────────────────────────┐
│  │  BOILER VITOCAL 100-S (97L)         │
│  │                                     │
│  │  ▲ Ingresso mandata (70°C)         │
│  │  │                                 │
│  │  │ ╔════════════════════════════╗ │
│  │  │ ║   SONDA ALTA (75°C max)   ║ │
│  │  │ ║   REG 104 (temp alta)     ║ │
│  │  │ ║   Attiva quando >65°C     ║ │
│  │  │ ╚════════════════════════════╝ │
│  │  │                                 │
│  │  │ ╔════════════════════════════╗ │
│  │  │ ║   SONDA MEDIA (45°C)       ║ │
│  │  │ ║   REG 106 (comfort)        ║ │
│  │  │ ║   Setpoint ACS: 45-50°C   ║ │
│  │  │ ╚════════════════════════════╝ │
│  │  │                                 │
│  │  │ ╔════════════════════════════╗ │
│  │  │ ║   SONDA BASSA (30°C min)   ║ │
│  │  │ ║   REG 107 (protezione)     ║ │
│  │  │ ║   Quando <25°C: allarme    ║ │
│  │  │ ╚════════════════════════════╝ │
│  │  │                                 │
│  │  │ ▼ Uscita ritorno (45°C)       │
│  │  │                                 │
│  └──────────────────────────────────────┘
│     │
│     └─→ [Pompa circolare notturna 10A]
│        └─→ [Pompa circolare diurna 10A]
│
└─ RITORNO → POMPA CALORE

DERIVAZIONI BOILER:
├─ ACS (Acqua Calda Sanitaria)
│  ├─ Uscita riscaldatore ausiliario (se <45°C)
│  └─ Pannello Solare SPCF pre-riscalda
│
└─ Circuito Fan-coil (via valvola 3-vie)
   ├─ Pompa di circolazione riscaldamento
   ├─ FAN-COIL 1 (Salotto) - Valvola zona 1
   └─ FAN-COIL 2 (Camera) - Valvola zona 2
```

---

## 🎛️ Controllo Elettrico

### Quadro EATON - Circuiti

![Quadro Elettrico EATON](../images/hardware/electrical/quadro-eaton.jpeg)

```
ALIMENTAZIONE: 230V/400V (Trifase)

┌─ GENERALE
│  └─ Interruttore principale 40A
│
├─ POMPA CALORE (20A - Circuito dedicato)
│  ├─ Contattore trifase
│  ├─ Protezione magnetotermica C20
│  └─ Capacitor banco (correzione fattore potenza)
│
├─ POMPA NOTTE (10A)
│  └─ Accesa 22:00-06:00 (riduzione consumi)
│
├─ POMPA GIORNO (10A)
│  └─ Accesa 06:00-22:00 (riscaldamento efficiente)
│
├─ SOLARE (C16)
│  └─ Circuito pannello SPCF 100L (protezione 16A)
│
├─ CALDAIA AUSILIARIA
│  └─ Backup riscaldamento (se pompa insufficiente)
│
└─ CIRCUITI AUSILIARI
   ├─ Luce locale
   ├─ Ventilconvettore elettrico/ventilatore
   └─ Protezione residuale 30mA (RCD)
```

---

## 📡 Sistema di Comunicazione - Modbus RTU

### RS485 Modbus RTU (9600 baud, 8N1)

```
TOPOLOGIA:
┌─ ESP32-S3 (Master)
│  └─ RS485 Module
│     └─ Cavo Twisted Pair Cat5e
│        └─ Viessmann Thermostat (Slave)
│
PARAMETRI:
• Velocità: 9600 baud
• Parità: None
• Stop bits: 1
• Data bits: 8
• Timeout: 3 secondi
• Retry: 6 tentativi (configurabile da Settings panel)
• Delay retry: 100ms (configurabile da Settings panel)

REGISTRI PRINCIPALI:
┌─────────┬──────────────────────────────┬──────────┬──────────┐
│ Registro│ Descrizione                  │ Range    │ Tipo     │
├─────────┼──────────────────────────────┼──────────┼──────────┤
│ REG 101 │ CONFIG (Power/Mode/Fan)      │ Bitmap   │ RW       │
│ REG 102 │ Temperatura setpoint (×10)   │ 5-35°C   │ RW       │
│ REG 103 │ Modalità stagionale (H/C)    │ 0-1      │ RW       │
│ REG 104 │ Temperatura alta boiler      │ 60-75°C  │ Read     │
│ REG 105 │ Temperatura mandata pompa    │ 35-70°C  │ Read     │
│ REG 106 │ Temperatura media boiler     │ 30-55°C  │ Read     │
│ REG 107 │ Temperatura bassa boiler     │ 10-30°C  │ Read     │
│ REG 108 │ Pressione bar                │ 0-4 bar  │ Read     │
└─────────┴──────────────────────────────┴──────────┴──────────┘

BITMAP REG 101 (CONFIG):
Bit 0:     Power (0=OFF, 1=ON)
Bit 1-2:   Fan speed (00=Auto, 01=Min, 10=Night, 11=Max)
Bit 13:    Modalità CALDO (1=Heating mode active)
Bit 14:    Modalità FREDDO (1=Cooling mode active)
Bit 15:    Allarme/Error (1=System error)

FLUSSO COMANDO MQTT:
1. HA pubblica su viessmann/set/temp = 21
2. ESP32 riceve da MQTT callback
3. Calcola: 21 × 10 = 210 (0x00D2)
4. Scrive REG 102 via Modbus RTU
5. Invia 6 tentativi a 100ms di intervallo
6. Legge feedback da REG 102
7. Pubblica su viessmann/temp = 21 (status)
8. Display locale si aggiorna (LVGL updateUI)
```

---

## 📲 Architettura Comunicazione Completa

```
LIVELLO 1: LOCALE (Rete Privata 192.168.0.x)
┌────────────────────────────────────────────────────┐
│                                                    │
│  ┌──────────────────────┐                         │
│  │   CRONOTERMOSTATO    │                         │
│  │   Touch 3.5" LCD     │                         │
│  │   (Controllo manuale)│                         │
│  │   IP: 192.168.0.89   │                         │
│  └──────────┬───────────┘                         │
│             │                                     │
│             │ RS485 Modbus RTU (Cablato)        │
│             │ 9600 baud, REG 101-108            │
│             ▼                                     │
│  ┌──────────────────────────────────────────┐   │
│  │   POMPA CALORE VIESSMANN VITOCAL 060     │   │
│  │   (Slave Modbus)                         │   │
│  │                                          │   │
│  │   Riceve:                                │   │
│  │   • Setpoint temperatura (REG 102)      │   │
│  │   • Modalità heat/cool (REG 103)        │   │
│  │   • Velocità ventola (REG 101)          │   │
│  │                                          │   │
│  │   Invia:                                 │   │
│  │   • Temperatura mandata (REG 105)       │   │
│  │   • Pressione (REG 108)                 │   │
│  │   • Status allarme (REG 101 bit 15)     │   │
│  └──────────────────────────────────────────┘   │
│             ▲                                     │
│             │                                     │
│             │ RS485 (da termostato)             │
│             │                                     │
│  ┌──────────┴──────────────────────┐            │
│  │   ESP32-S3 Touch LCD 4.3"       │            │
│  │   (Controller Intelligente)      │            │
│  │   IP: 192.168.0.89               │            │
│  │                                  │            │
│  │   Funzioni:                      │            │
│  │   • Display locale real-time     │            │
│  │   • Controllo touch manuale      │            │
│  │   • Sleep timeout 30-600s        │            │
│  │   • Settings panel (⚙️)           │            │
│  └──────────┬───────────────────────┘            │
│             │                                     │
│             │ WiFi 2.4GHz 802.11 b/g/n          │
│             │                                     │
│  ┌──────────▼───────────────────────┐            │
│  │   RASPBERRY PI (Locale HA)       │            │
│  │   IP: 192.168.0.86                │            │
│  │   OS: Raspberry Pi OS             │            │
│  │                                  │            │
│  │   ├─ Home Assistant (8123)        │            │
│  │   │  └─ Dashboard visuale         │            │
│  │   │  └─ Automazioni orarie        │            │
│  │   │  └─ Storico/Grafici           │            │
│  │   │                              │            │
│  │   └─ Mosquitto MQTT (1883)        │            │
│  │      └─ Broker locale             │            │
│  │      └─ Topics viessmann/*        │            │
│  │      └─ Subscriptions MQTT        │            │
│  └──────────────────────────────────┘            │
│                                                    │
└────────────────────────────────────────────────────┘

LIVELLO 2: REMOTO (Internet - Cloudflare Tunnel)
┌────────────────────────────────────────────────────┐
│                                                    │
│  ┌──────────────────────────────────────────┐    │
│  │   CLOUDFLARE TUNNEL                      │    │
│  │   (VPN tunnel sicuro, zero exposures)    │    │
│  │                                          │    │
│  │   URL pubblico:                          │    │
│  │   https://casa.tunnelname.com            │    │
│  │                                          │    │
│  │   Interno:                               │    │
│  │   192.168.0.86:8123 (HA solo)           │    │
│  │   (Mosquitto NON esposto)                │    │
│  └──────────────────────────────────────────┘    │
│             │                                     │
│             │ TLS/HTTPS tunnel                   │
│             │                                     │
│  ┌──────────▼──────────────────────┐             │
│  │   APP MOBILE / WEB               │             │
│  │   (Controllo remoto da ovunque)  │             │
│  │                                  │             │
│  │   • Home Assistant Cloud         │             │
│  │   • Accesso remoto sicuro        │             │
│  │   • Notifiche push               │             │
│  │   • Storico temperatura          │             │
│  │   • Automazioni programmabili    │             │
│  └──────────────────────────────────┘             │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 🔄 Flusso Comando Completo - Esempio Pratico

### Scenario: Cambio temperatura da 20°C a 22°C via Home Assistant

```
STEP 1: Utente interagisce con Dashboard HA (da cellulare remoto)
────────────────────────────────────────────────────────────────
  [HA Cloud App]
       │
       │ Utente tocca slider da 20°C a 22°C
       │
       ▼
  [Cloudflare Tunnel] → [Raspberry Pi:8123] → [Home Assistant]
       │
       │ HA legge il nuovo setpoint
       │
       ▼ MQTT Publish
  Mosquitto (192.168.0.86:1883)
  Topic: viessmann/set/temp
  Payload: 22

STEP 2: ESP32 riceve comando MQTT
────────────────────────────────────────────────────────────────
  [ESP32 MQTT Client]
       │
       ├─ Connessione: 192.168.0.86:1883
       ├─ Username: viessmann (se configurato)
       │
       ├─ Subscribe a: viessmann/set/#
       │
       ▼
  mqttCallback() viene chiamato
       │
       ├─ Topic ricevuto: viessmann/set/temp
       ├─ Payload: "22"
       │
       ├─ globalTempSetpoint = 22
       │
       ├─ Converti: 22 × 10 = 220 (0x00DC)
       │
       └─→ Scrivi REG 102 = 0x00DC

STEP 3: Modbus RTU → Viessmann (con retry logic)
────────────────────────────────────────────────────────────────
  [sendAllRegisters() function]
       │
       ├─ RETRY LOOP (max 6 tentativi):
       │  
       │  Tentativo 1:
       │    └─ RS485 TX: "REG 101 = 0xC001, REG 102 = 0x00DC, REG 103 = 0x0001"
       │       (9600 baud, Modbus ASCII)
       │    └─ Attesa risposta: 3 secondi
       │    └─ Se timeout → Tentativo 2
       │
       │  Tentativo 2:
       │    └─ Ripeti trasmissione
       │    └─ Delay 100ms tra tentativi (configurabile)
       │
       │  ...continua fino a successo o max 6 tentativi
       │
       └─→ Timeout/Risposta ricevuta
              │
              ├─ RS485 RX: Ack dalla pompa
              └─ Registri aggiornati nella pompa

STEP 4: Lettura feedback Modbus
────────────────────────────────────────────────────────────────
  [Polling loop]
       │
       ├─ Ogni 10 secondi, leggi REG 102-108 da pompa
       │
       ├─ REG 102 = 220 (22°C) ✓
       ├─ REG 105 = mandata pompa
       ├─ REG 108 = pressione
       │
       └─→ Valori aggiornati in memoria globale

STEP 5: Display locale si aggiorna (LVGL)
────────────────────────────────────────────────────────────────
  [updateUI() function]
       │
       ├─ LVGL Lock
       │
       ├─ Aggiorna label temperatura: "22.0°C"
       ├─ Aggiorna sonda display
       ├─ Aggiorna indicator mandata (rosso/blu)
       │
       └─ LVGL Unlock
              │
              ▼
           [DISPLAY TOUCH ESP32]
           Mostra: 22.0°C ✅

STEP 6: ESP32 pubblica status su MQTT
────────────────────────────────────────────────────────────────
  [mqttPublishStatus() - ogni 10 secondi]
       │
       │ Publish JSON su: viessmann/status
       │
       ▼
  {
    "timestamp": "2026-04-12T14:32:15Z",
    "power": true,
    "temperature": 22.0,
    "fan_speed": 3,
    "mode": "heat",
    "modbus_status": "OK",
    "boiler_temp": 48.2,
    "return_temp": 35.8,
    "pressure_bar": 2.4
  }
       │
       └─→ Mosquitto riceve e distribuisce
              │
              └─→ Home Assistant subscribe e aggiorna

STEP 7: Home Assistant riceve feedback
────────────────────────────────────────────────────────────────
  [Home Assistant MQTT Integration]
       │
       ├─ Topic: viessmann/temp = 22
       ├─ Topic: viessmann/status = {...}
       │
       ├─ Aggiorna sensori nel dashboard
       ├─ Aggiorna storico/database
       ├─ Aggiorna grafici
       │
       └─→ Cloud HA riceve via Cloudflare tunnel

STEP 8: Feedback visuale all'utente
────────────────────────────────────────────────────────────────
  [App Mobile / Web HA]
       │
       ├─ Dashboard mostra: 22°C ✅
       ├─ Storico aggiornato
       ├─ Pressure: 2.4 bar ✅
       │
       └─→ COMPLETATO
```

---

## 🌡️ Modalità Operative

### INVERNO - Riscaldamento Attivo

```
Setpoint: 18-22°C
Modalità: CALDO (bit 13=1, bit 14=0)
Pompa: 6kW riscaldamento

Flow:
1. Pompa estrae calore da aria esterna (-5°C)
2. Compressore attivato
3. Boiler mantiene 50°C (ACS)
4. Ventilconvettori riscaldano a 55°C
5. Fan-coil circolano aria calda negli ambienti
6. Ritorno: 45°C

Consumo: ~2-3 kW/h (COP 2.5-3.0)
Efficienza: 300% (3x energia in riscaldamento vs energia elettrica)
```

### ESTATE - Raffrescamento Attivo

```
Setpoint: 26-28°C
Modalità: FREDDO (bit 14=1, bit 13=0)
Pompa: 5kW raffrescamento

Flow:
1. Pompa inverte ciclo frigorifero
2. Estrae calore da ambienti (28°C)
3. Scarica calore all'esterno (35°C)
4. Ventilconvettori raffreddano a 12-15°C
5. Condensa gestita in scarico
6. Circuito ACS disattivato

Consumo: ~2-2.5 kW/h
Efficienza: Coefficiente di efficienza 2.0-2.2
Nota: Più inefficiente del riscaldamento (delta termico minore)
```

### NOTTE - Modalità Ridotta

```
Ore: 22:00 - 06:00 (programmabile)
Fan speed: Minimo (velocità ridotta)
Temperatura: -2/-3°C dal setpoint
Pompa notte: 10A (efficienza energetica)

Flow:
1. Compressore modulato
2. Ventola a velocità minima
3. Rumore ridotto (<35dB)
4. Comfort mantenuto
5. Consumi al minimo

Vantaggi: -40% consumo energetico
Rumore: Quasi silenzioso
```

---

## 📊 Monitoraggio e Allarmi

### Tabella Limiti Operativi

| Parametro | Range OK | Giallo (Attenzione) | Rosso (Allarme) | Azione |
|-----------|----------|--------------------|--------------------|--------|
| **Pressione pompa** | 2.0-3.0 bar | 1.5-1.9 / 3.1-3.5 | <1.5 o >3.5 | Allarme HA + Email |
| **Temp mandata (inv)** | 35-65°C | 65-70°C | >70°C | Spegnimento pompa |
| **Temp ritorno (inv)** | 25-50°C | 50-55°C | >55°C | Riduzione velocità |
| **Temp boiler alta** | 40-65°C | 65-70°C | >75°C | Protezione termostato |
| **Temp boiler media** | 40-55°C | 55-60°C | <30°C o >60°C | Allarme sensore |
| **Pressione bar (boiler)** | 2.0-3.0 | 1.5-1.9 / 3.1-3.5 | <1.2 o >4.0 | Reset manuale |
| **Umidità ambiente** | 40-60% | 30-39% / 61-70% | <20% o >75% | Avviso display |
| **COP reale** | 2.5-3.0 | 1.5-2.4 | <1.0 | Anomalia sistema |

### Codici Allarme (REG 101 bit 15)

```
0x0001 = Pressione troppo bassa (<1.5 bar)
0x0002 = Temperatura mandata eccessiva (>70°C)
0x0004 = Sensore pressione disconnesso
0x0008 = Sensore temperatura guasto
0x0010 = Compressore in protezione termica
0x0020 = Comunic. Modbus perduta (timeout >3s)
0x0040 = Blocco sicurezza (ciclo di 3 min)
0x0080 = Saturazione olio compressore
```

---

## 🔐 Parametri Configurabili (Settings Panel ⚙️)

Accessibili direttamente dal **Display Touch ESP32**:

```
┌─────────────────────────────────────────────────┐
│              ⚙️ SETTINGS PANEL                  │
├─────────────────────────────────────────────────┤
│                                                 │
│  1. MODBUS RETRIES              [━━●━━━━] 6     │
│     Range: 1-10 (step 1)                       │
│     Effetto: Aumenta affidabilità (↑ ritardi) │
│                                                 │
│  2. MODBUS RETRY DELAY          [━━━●━━] 100ms │
│     Range: 0-1000ms (step 50ms)               │
│     Effetto: Spazio tra tentativi             │
│                                                 │
│  3. DISPLAY SLEEP TIMEOUT       [━━●━━━] 180s  │
│     Range: 30-600s (step 30s)                 │
│     Effetto: Spegni schermo dopo inattività   │
│     Timer reset al tocco durante awake        │
│                                                 │
│  [Salva]  [Annulla]                           │
│                                                 │
└─────────────────────────────────────────────────┘

SALVATAGGIO:
• NVS Flash memory ESP32
• Persistente dopo riavvio
• Backup ogni cambio
```

---

## 🚀 Integrazione Home Assistant Completa

### Dashboard HA (192.168.0.86:8123)

```
SENSORI MQTT:
├─ viessmann/power        → Switch ON/OFF
├─ viessmann/temp         → Sensor (float °C)
├─ viessmann/fan          → Select (Auto/Min/Notte/Max)
├─ viessmann/mode         → Select (heat/cool/off)
├─ viessmann/status       → JSON string (completo)
├─ viessmann/pressure     → Sensor (bar)
├─ viessmann/boiler_temp  → Sensor (°C)
└─ viessmann/return_temp  → Sensor (°C)

AUTOMAZIONI CONSIGLIATE:
├─ Accensione mattina (06:00): heat 21°C
├─ Spegnimento notte (22:00): off
├─ Raffrescamento estate (1 giu): cool 26°C
├─ Notifiche allarme pressione
└─ Grafico trend energia (kWh)

NOTIFICHE:
├─ Allarme pressione bassa
├─ Temperatura mandata eccessiva
├─ Comunicazione Modbus perduta
└─ Sistema offline >5 minuti
```

---

## 📞 Supporto Tecnico

- **Documentazione Modbus**: `/docs/MODBUS.md`
- **Guida MQTT**: `/docs/MQTT.md`
- **Tunnel Remoto**: `/docs/CLOUDFLARE.md`
- **Viessmann Support**: https://www.viessmann.it
- **Firmware OTA**: README.md → Sezione Update

---

*Documento completo: Aprile 2026*
*Sistema Viessmann Intelligente - Casa Molinella*
*Controllore: Francesco Dal Savio*
