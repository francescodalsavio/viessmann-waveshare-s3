# Cronotermostato LCD Modbus Viessmann

## Introduzione

Il **Cronotermostato LCD Modbus** è il dispositivo di controllo programmabile per gestire i ventilconvettori Energycal Slim. È compatibile con il nostro sistema ESP32-S3 Touch Display tramite protocollo Modbus RS485.

---

## Caratteristiche Principali

### Gestione Ventilconvettori
- **Fino a 30 terminali** gestibili contemporaneamente
- Regolazione modulante della velocità in modalità automatica
- Massimo comfort con massima efficienza

### Display e Controllo
- **Display LCD retroilluminato** — leggibile in tutte le condizioni
- **Programmazione a fasce orarie settimanali** — automazione giornaliera
- **Sonda di temperatura ambiente integrata** — controllo automatico

### Protocollo Comunicazione
- **Modbus RS485** — porta AB per integrazione domotica
- **Indirizzamento**: Fino a 30 dispositivi sulla stessa linea
- **Baud rate**: 9600 baud (standard Modbus)
- **Formato**: ASCII RS485

### Compatibilità
- ✅ Compatibile con **comando universale Touch** Energycal Slim
- ✅ Compatibile con **ESP32-S3 Touch Display** (nostro sistema)
- ✅ Integrabile in sistemi domotici via Modbus

---

## Funzionamento

### Modalità di Controllo

#### 1. Modulante Automatica
- Sonda temperatura ambiente regola continuamente la velocità ventilatore
- Temperatura raggiunge setpoint con efficienza massima
- Riduce consumi energetici

#### 2. Programmazione Settimanale
- **Fasce orarie**: Definibili per ogni giorno della settimana
- **Automazione**: Accensione/spegnimento automatico per ora
- **Setpoint temperatura**: Programmabile per ogni fascia

**Esempio di programmazione:**
```
LUNEDÌ-VENERDÌ:
  06:00-22:00: 22°C (giorno)
  22:00-06:00: 16°C (notte)

SABATO-DOMENICA:
  08:00-23:00: 20°C
  23:00-08:00: 18°C
```

#### 3. Gestione Multi-Terminale
- Ogni ventilconvettore ha un indirizzo Modbus univoco (1-30)
- Cronotermostato comunica singolarmente con ogni terminale
- Setpoint temperatura uguale per tutti (o differenziabile via domotica)

### Sonda Temperatura Integrata

La sonda misura continuamente la temperatura ambiente e:
- Regola la velocità ventilatore per mantenere il setpoint
- Arresta il ventilatore se temperatura raggiunge setpoint
- Riavvia se temperatura scende sotto setpoint

---

## Interfaccia Modbus RS485

### Connessione Fisica

```
Cronotermostato ← RS485 → Ventilconvettore (comando Touch)
                    ↓
                 ESP32-S3
                (nostro sistema)
```

### Parametri Modbus

| Parametro | Valore |
|-----------|--------|
| **Protocollo** | Modbus ASCII RS485 |
| **Baud rate** | 9600 baud |
| **Data bits** | 8 |
| **Stop bits** | 1 |
| **Parity** | Nessuna (N) |
| **Indirizzo dispositivo** | 1-30 (broadcast: 0) |
| **Porta**: Morsetti AB | Standard Modbus RS485 |

### Comandi Modbus Supportati

Il cronotermostato supporta i seguenti comandi Modbus:

#### 1. Lettura Temperatura Ambiente
```
Read Input Registers (funzione 0x04)
Registro: Temperatura ambiente (°C × 10)
Esempio: 0x00D2 = 21.0°C
```

#### 2. Lettura Stato Terminale
```
Read Input Registers (funzione 0x04)
Registri: Stato ON/OFF, velocità ventilatore, setpoint
```

#### 3. Impostazione Setpoint Temperatura
```
Write Single Register (funzione 0x06)
Registro: Temperatura setpoint (°C × 10)
Range: 16-28°C (standard), estensibile a 5-40°C
```

#### 4. Controllo Velocità Ventilatore
```
Write Single Register (funzione 0x06)
Valori: 0=OFF, 1=MIN, 2=MEDIO, 3=MAX
```

#### 5. Accensione/Spegnimento
```
Write Single Register (funzione 0x06)
Valore: 1=ON, 0=OFF
```

---

## Display e Menu

### Schermata Principale
```
┌─────────────────────────┐
│ TEMP:    22.5°C         │
│ SETP:    22.0°C         │
│ FAN:     3 (MAX)        │
│ STATO:   RISCALDO       │
└─────────────────────────┘
```

### Menu Programmazione
1. **Orari**: Impostazione fasce orarie settimanali
2. **Setpoint**: Temperatura per fascia oraria
3. **Velocità**: Velocità ventilatore (auto/manuale)
4. **Dispositivi**: Assegnazione indirizzi Modbus
5. **Visualizzazione**: Luminosità, timeout display

---

## Programmazione Settimanale

### Passi di Programmazione

1. **Accesso Menu**: Premere "MENU" sul display
2. **Selezionare "PROGRAMMA"**
3. **Scegliere giorno della settimana**
4. **Definire fasce orarie**:
   - Ora inizio
   - Ora fine
   - Temperatura setpoint
   - Velocità ventilatore (auto/manuale)
5. **Salvare**

### Limiti di Programmazione

- **Massimo 8 fasce per giorno**
- **Setpoint minimo**: 16°C (anticongelamento)
- **Setpoint massimo**: 28°C (standard), 40°C (esteso)
- **Precisione temperatura**: ±0.5°C
- **Aggiornamento**: Ogni 30 secondi

---

## Integrazione con Nostro Sistema

### Architettura

```
┌──────────────────────┐
│  Cronotermostato LCD │ (Modbus Master)
│  (programmazione)    │
└──────────┬───────────┘
           │ Modbus RS485
           ↓
     ┌─────────────┐
     │  RS485 Bus  │
     └──┬──────┬──┬┘
        │      │  └──── Ventilconvettore 3
        │      └─────── Ventilconvettore 2
        │
        └──────────────  Ventilconvettore 1 (maestro)
                         + Comando Touch
                         + ESP32-S3 (nostro)
```

### Nostro Ruolo (ESP32-S3)

Il nostro **ESP32-S3 Touch Display**:
1. **Ascolta** il bus Modbus RS485 (sniffer passivo)
2. **Intercetta** comandi del cronotermostato verso i ventilconvettori
3. **Fornisce** interfaccia web per controllo alternativo
4. **Memorizza** stato e temperature per analytics

### Comunicazione Modbus

```
Cronotermostato → Ventilconvettore 1 (addr 1)
  :01 06 0065 4003 [LRC] \r\n
  └─ Set REG 101 = 0x4003 (ON)

ESP32 (passivo)
  Legge frame completo dal bus
  Aggiorna stato locale
  Espone via web API
```

---

## Funzionalità Avanzate

### Controllo Modulante
- **Setpoint**: 22°C
- **Temperatura attuale**: 21°C
- **Differenza**: -1°C
- **Azione**: Ventilatore a velocità ridotta (invece di ON/OFF)
- **Beneficio**: Comfort maggiore, consumi minori

### Contatto Finestra (Opzionale)
- Se connesso: rileva apertura finestra
- Azione automatica: Spegne ventilatore
- Riattiva quando finestra chiude

### Sonda Acqua Esterna (Opzionale)
- Controlla temperatura minima/massima dell'acqua
- Protezione da congelamento (inverno)
- Protezione da surriscaldamento (estate)

---

## Manutenzione

### Batteria
- Tipo: Batteria backup (opzionale)
- Funzione: Mantiene programmazione in caso di blackout
- Durata: 1-2 anni

### Sonda Temperatura
- Controllo: Verificare che sia pulita e libera da ostacoli
- Posizionamento: 1.5m dal suolo, lontano da fonti di calore/freddo
- Calibrazione: Regolabile via menu (+/-3°C)

### Display
- Retroilluminazione: Spegnimento automatico dopo 1 minuto (configurabile)
- Contrasto: Regolabile manualmente
- Pulizia: Panno umido, evitare liquidi

---

## Troubleshooting

### Display Spento
✅ Verificare alimentazione (230V AC)  
✅ Controllare fusibile (se presente)  
✅ Ripristinare alimentazione (10 secondi)

### Ventilconvettore Non Risponde
✅ Verificare cablaggio RS485 (A, B, GND)  
✅ Controllare indirizzo Modbus (menu dispositivi)  
✅ Verificare baud rate (9600)  
✅ Controllare che comando Touch sia in modalità Modbus

### Temperatura Non Cambia
✅ Verificare sonda integrata (posizione, contatti)  
✅ Controllare setpoint (display)  
✅ Verificare programma settimanale (orario corrente attivo?)  
✅ Verificare che ventilatore sia alimentato

### Programmazione Non Salvata
✅ Verificare batteria backup (se presente)  
✅ Riprogrammare le fasce orarie  
✅ Salvare esplicitamente (pulsante SAVE)

---

## Specifiche Tecniche

| Parametro | Valore |
|-----------|--------|
| **Alimentazione** | 230V AC, 50Hz |
| **Consumo** | <5W |
| **Display** | LCD retroilluminato, 2-3 righe |
| **Sonda temperatura** | NTC 10kΩ ±0.5°C |
| **Precisione setpoint** | ±0.5°C |
| **Fasce orarie** | 8 per giorno (56 al massimo) |
| **Terminali gestibili** | 30 (indirizzi Modbus 1-30) |
| **Modbus baud rate** | 9600 baud |
| **Porta comunicazione** | Morsetti AB (RS485) |
| **Tensione uscita relè** | 230V AC max 16A (per elettrovalvole) |
| **Temperatura funzionamento** | 0-50°C |
| **Umidità** | 10-90% (non condensante) |
| **Dimensioni** | ~180×90×40 mm |
| **Peso** | ~200g |

---

## Vantaggi del Sistema

✅ **Programmazione settimanale** — Automazione completa  
✅ **Modulazione continua** — Massimo comfort e efficienza  
✅ **Multi-terminale** — Gestisce fino a 30 ventilconvettori  
✅ **Sonda integrata** — Temperatura ambiente in tempo reale  
✅ **Modbus RS485** — Standard industriale  
✅ **Compatibile ESP32** — Integrazione con nostro sistema  
✅ **Display intuitivo** — Programmazione semplice  

---

## Riferimenti

- **Produttore**: Viessmann
- **Compatibilità**: Energycal Slim (tutti i modelli)
- **Protocollo**: Modbus ASCII RS485 (IEC 61131-3)
- **Documentazione**: Disponibile su viessmann.it
