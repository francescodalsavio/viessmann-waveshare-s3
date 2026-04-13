# Vitoconnect (OPZIONALE) ⚠️

> **Fonte**: Manuale ufficiale Viessmann 5834165 IT 5/2019 → `viessmann_istruzioni_montaggio_utilizzo_vitoconnect.PDF`

---

## ⚠️ NON FA PARTE DI QUESTO PROGETTO

**Questo progetto (viessmann-waveshare-s3) NON usa Vitoconnect.**

| | **Questo Progetto** | **Vitoconnect** |
|---|---|---|
| **Controlla** | Ventilconvettori Energycal Slim W | Vitocal 100-S/111-S (pompa di calore) |
| **Protocollo** | Modbus ASCII RS485 | Optolink → Cloud Viessmann |
| **Hardware** | ESP32-S3 + FT232RNL | Modulo WiFi dedicato |
| **Internet** | Non richiesto | Obbligatorio |
| **Costo** | ~€20 (FT232RNL) | ~€210 (Vitoconnect) |

**Sono due sistemi completamente separati:**
- **Questo progetto** = crontermostatoi con touch screen per ventilconvettori fan-coil
- **Vitoconnect** = controllo remoto cloud per la pompa di calore tramite App ViCare

---

## Cos'è Vitoconnect?

**Vitoconnect** è un **modulo WiFi** ufficiale Viessmann che fa da ponte tra la pompa di calore (via Optolink) e l'App ViCare su smartphone via cloud.

Esistono due versioni (dal manuale ufficiale):
- **OPTO2** → per impianti con interfaccia **Optolink** (Vitocal 100-S/111-S ✅)
- **OT2** → per impianti con interfaccia **Open Therm** (Vitodens, ecc.)

### Schema Collegamento (dal manuale ufficiale)

```
VITOCAL 100-S/111-S
    │
    │ Cavo Optolink (3m, incluso)
    ▼
VITOCONNECT 100 OPTO2
    │  ├─ Tasto di comando (A)
    │  ├─ Tasto Reset (B)
    │  ├─ LED Casa    → stato connessione generatore
    │  ├─ LED WiFi    → stato comunicazione WLAN/server
    │  └─ LED Ruota   → funzioni sistema/generatore
    │
    │ WiFi 2.4GHz (WPA2)
    ▼
Router WLAN (internet flat sempre attivo)
    │
    │ Internet
    ▼
Server Viessmann (cloud)
    │
    ▼
App ViCare (iOS/Android)
oppure
App Vitoguide (solo centro assistenza autorizzato)
```

---

## Protocolli di Comunicazione Viessmann

L'impianto Viessmann usa **tre protocolli diversi** per tre scopi diversi. È importante non confonderli:

```
IMPIANTO COMPLETO
│
├── POMPA DI CALORE Vitocal 100-S
│   │
│   ├── Optolink (proprietario Viessmann)
│   │   └── Usato da: Vitoconnect OPTO2 → Cloud → App ViCare
│   │
│   └── RS485 Modbus RTU (standard industriale)
│       └── Usato da: sistemi BMS industriali, integrazione futura
│           Connettore: RJ45 sul Vitotronic 200
│           ⚠️ PORTA SEPARATA da Optolink
│
└── VENTILCONVETTORI Energycal Slim W
    └── RS485 Modbus ASCII (standard industriale)
        └── Usato da: questo progetto (ESP32-S3)  ← IL NOSTRO PROGETTO
```

### Cos'è Optolink?

**Optolink** è un protocollo **proprietario Viessmann** che usa un'interfaccia fisica ottica (infrarossi) o USB per comunicare con le centraline Viessmann. **Non è Modbus**.

- Usato **solo** da dispositivi Viessmann ufficiali (Vitoconnect, tool diagnostici)
- Non documentato pubblicamente
- Non accessibile senza hardware Viessmann dedicato

### Esiste Modbus sulla Vitocal?

**Sì**, il Vitotronic 200 nella Vitocal 100-S ha una **porta RS485 Modbus RTU separata** (connettore RJ45), pensata per sistemi BMS industriali.

- È una porta **completamente separata** da Optolink
- Protocollo **standard e documentato** (Modbus RTU)
- Potenziale integrazione futura con Home Assistant (separato da questo progetto)

### Riepilogo Protocolli

| Protocollo | Standard | Usato per | Chi lo usa |
|-----------|---------|-----------|-----------|
| **Optolink** | Proprietario Viessmann | Controllo remoto Vitocal via cloud | Vitoconnect → ViCare App |
| **Modbus RTU RS485** | Standard industriale aperto | BMS / integrazione Vitocal locale | Futuro: Home Assistant diretto |
| **Modbus ASCII RS485** | Standard industriale aperto | Controllo ventilconvettori | **Questo progetto (ESP32-S3)** |

---

## Acquisto (Amazon IT)

- **ASIN**: B07SYPYN48
- **Prezzo attuale**: ~€209,99 (Aprile 2026)
- **Venditore**: Serdar Catak / Amazon
- **Valutazione**: 4,5/5 stelle (260 recensioni)
- **#1 più venduto** in categoria "Controlli HVAC"

> ⚠️ **ATTENZIONE - Recensione italiana negativa:**
> *"Prodotto non destinato al mercato italiano ma tedesco, in Italia non funziona"* — Luca Marchese, Marzo 2025
>
> Verificare prima dell'acquisto la compatibilità con il modello specifico di Vitocal.

---

## Dati Tecnici (dal manuale ufficiale)

| Parametro | Valore |
|-----------|--------|
| **Tensione nominale** | 12V DC |
| **Corrente nominale** | 0,5A |
| **Potenza assorbita** | 5,5W |
| **Frequenza WLAN** | 2,4 GHz |
| **Codifica WLAN** | WPA2 (o non crittografata) |
| **Banda frequenza** | 2400,0 – 2483,5 MHz |
| **Max potenza TX** | 0,1W (e.i.r.p.) |
| **Protocollo Internet** | IPv4 / DHCP |
| **Classe protezione** | III / IP20D |
| **Temp. funzionamento** | +5 a +40°C |
| **Temp. deposito** | −20 a +60°C |
| **Alimentatore** | 100–240V~ / 50–60Hz → 12V out |
| **Distanza dal generatore** | min. 0,3m – max. 2,5m |
| **Cavo Optolink** | 3m (incluso) |
| **Cavo alimentazione** | 1,5m (incluso) |
| **Certificazione** | VDE Smart Home + RoHS 2011/65/EU |

---

## Regolazioni Supportate (dal manuale)

**Per pompe di calore (questo caso):**
- Vitotronic 200, tipo **WO1A, WO1B, WO1C** ← Vitocal 100-S usa WO1C

**Per caldaie murali:**
- Vitotronic 200, tipo HO1, HO1A, HO1B, HO1D, HO2B, RF HO1C/HO1E

**Per caldaie a basamento:**
- Vitotronic 200, tipo KO1B, KO2B, KW6, KW1, KW2, KW4, KW5
- Vitotronic 300, tipo KW3

**Per caldaie a legna (solo OPTO2):**
- Vitoligno 300-C, 200-S, 250-S, 300-S con Ecotronic

---

## Requisiti di Sistema (dal manuale)

### Rete
- ✅ Router WLAN con **WPA2** (no connessioni non crittografate)
- ✅ Connessione Internet **"abbonamento flat"** sempre attiva
- ✅ IP dinamico (**DHCP**)
- ✅ Porte aperte in uscita: **80, 123, 443, 8883**
- ⚠️ **Consigliato UPS** per continuità in caso di blackout

### Luogo di montaggio
- Solo **interno** (asciutto, no gelo)
- Temperatura: **+5 a +40°C**
- Distanza generatore: **0,3m – 2,5m**
- Presa Schuko 230V/50Hz vicina (max 1,5m)
- Segnale WLAN sufficiente (amplificabile con ripetitore)

### Account
- **Profilo utente Viessmann obbligatorio** (registrazione via ViCare App)

---

## Installazione (Procedura dal Manuale)

### Sequenza Montaggio

| Step | Operazione | Chi |
|------|-----------|-----|
| 1 | Controllo requisiti di sistema | IT / Tecnico |
| 2 | Montaggio supporto a parete | Utente |
| 3 | Collegamento Optolink al generatore | Tecnico |
| 4 | Inserimento nel supporto | Utente |
| 5 | Allacciamento rete 230V | Utente |
| 6 | Registrazione utente + configurazione WLAN | Utente |

### Allacciamento OPTO2 (Fig. 6-7 manuale)
1. Inserire **spina USB** del cavo Optolink nell'allacciamento **A** del Vitoconnect
2. Collegare la **spina Optolink** all'interfaccia Optolink sulla regolazione generatore

### Configurazione WiFi

**Android**: Connessione automatica a WLAN `VITOCONNECT-<xxxx>`

**iOS**:
1. Impostazioni → WiFi → `VITOCONNECT-<xxxx>`
2. Inserire password WPA2 (copiata automaticamente negli appunti)
3. Tornare all'App ViCare

### Verifica Completamento
- **LED bianco acceso** = OK, tutto configurato correttamente
- **LED rosso acceso** = Connessione non riuscita (vedi troubleshooting)

---

## Significato LED (dal manuale)

### LED Casa (stato connessione)
| Stato LED | Significato |
|-----------|-------------|
| Bianco acceso | ✅ Connessione OK con generatore e server |
| Bianco lampeggia | Funzionamento comfort attivo (solo OT2) |

### LED WiFi (comunicazione WLAN/server)
| Stato LED | Significato |
|-----------|-------------|
| Verde lampeggia | Connessione con server Viessmann in corso |
| Verde acceso | Vitoconnect si sta accendendo / Access Point attivo |
| Giallo pulsa | ⚠️ Connessione con router WLAN non riuscita |
| Giallo acceso | ⚠️ No connessione Internet |
| Rosso acceso | ⚠️ Connessione con server Viessmann non riuscita |

### LED Ruota (sistema/generatore)
| Stato LED | Significato |
|-----------|-------------|
| Verde acceso | Accensione / aggiornamento software in corso |
| Rosso lampeggia | Pronto per ripristino fabbrica |
| Rosso acceso | ⚠️ Errore nella regolazione generatore |
| Rosso + WiFi lampeggia veloce | ⚠️ Errore software/hardware |

---

## Troubleshooting (dal manuale)

| Problema | Causa | Soluzione |
|----------|-------|-----------|
| LED giallo lampeggia | WLAN non connessa | Verifica password, riavvia |
| LED giallo acceso | No internet | Controlla router/internet |
| LED rosso acceso | No server Viessmann | Controlla internet, riavvia |
| LED rosso veloce | Errore SW/HW | Riavvia, sostituisci se persiste |
| LED giallo veloce | No connessione generatore | Controlla cavi Optolink/USB |
| Nessun LED | No alimentazione | Controlla presa 230V |

### Riavvio
Premi **Reset + Tasto Comando** simultaneamente per **15 secondi**.

### Ripristino Fabbrica
1. Premi **Reset** per **min. 30 secondi** → LED rosso lampeggia
2. Ripremi **Reset** entro 5 secondi → Vitoconnect si riavvia

---

## Funzioni Disponibili (via App ViCare)

✅ Controllo setpoint temperatura  
✅ Programmazione fasce orarie (giorno/notte)  
✅ Modalità comfort temporanea (+1/-1°C per max 24h)  
✅ Funzione vacanza  
✅ Monitoraggio stato impianto  
✅ Notifiche push / email  
✅ Manutenzione remota (Vitoguide, solo tecnico autorizzato)  
✅ Aggiornamenti software automatici

---

## Manutenzione

- **Nessuna manutenzione** richiesta (esente da manutenzione)
- Pulizia: **panno microfibra** (no detergenti)
- Aggiornamenti firmware: **automatici** (LED verde durante update, max 5 min)

---

## Confronto con Approccio DIY di Questo Progetto

| Aspetto | **Vitoconnect** | **FT232RNL + ESP32** |
|---------|----------|---------|
| **Cosa controlla** | Pompa di calore (Vitocal) | Ventilconvettori (fan-coil) |
| **Protocollo** | Optolink (proprietario) | Modbus ASCII RS485 |
| **Internet** | Obbligatorio | Non necessario |
| **Latenza** | ~60s (polling cloud) | ~100ms (locale) |
| **Automazioni** | Limitate (ViCare) | Illimitate (Home Assistant) |
| **Costo** | ~€100 | ~€20 (FT232RNL) |
| **Privacy** | Dati in cloud Viessmann | 100% locale |
| **Offline** | Non funziona | Funziona sempre |

---

## File in questa Cartella

```
vitoconnect_(optional)/
├── README.md                                              ← Questo file
└── viessmann_istruzioni_montaggio_utilizzo_vitoconnect.PDF ← Manuale ufficiale Viessmann 5834165 IT 5/2019
```

---

**Fonte**: Manuale Viessmann Vitoconnect 5834165 IT 5/2019  
**Aggiornamento**: Aprile 2026
