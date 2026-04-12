# Boiler di Accumulo Termico - Vitocal 100-S

## Panoramica

**Vitocal 100-S** - Bollitore di accumulo termico multistrato per pompe di calore aria-acqua. Buffer termico per riscaldamento, raffrescamento e acqua calda sanitaria.

## Specifiche Tecniche

| Parametro | Valore |
|-----------|--------|
| **Modello** | Viessmann Vitocal 100-S |
| **Tipo** | Boiler di accumulo multistrato |
| **Capacità totale** | 97 litri |
| **Isolamento termico** | Multistrato ad altissime prestazioni |
| **Pressione nominale** | 3 bar |
| **Pressione max** | 4 bar |
| **Temperatura protezione** | Termostato 60°C |
| **Sonde temperatura** | 3 sonde (alta, media, bassa) |
| **Manometro** | Analogico per lettura pressione |
| **Anodo protettivo** | Magnesio (protezione corrosione) |
| **Collegamento** | Attacchi 1" per mandata/ritorno |
| **Isolamento spessore** | ~100mm (R>3) |
| **Colore** | Bianco |

## Come Si Riscalda il Boiler? (Solare vs Pompa di Calore)

**Domanda**: Quando il Vitotronic 200 imposta il riscaldamento a 22°C, come porta il boiler a 55°C? Usa il pannello solare o la pompa di calore?

**Risposta**: **Dipende!** Il sistema ha una **logica di priorità intelligente**:

### Logica di Priorità - Chi Riscalda il Boiler?

```
VITOTRONIC CONTROLLA T_BOILER COSTANTEMENTE:

Se T_boiler < 55°C (insufficiente):
   │
   ├─ CONTROLLO SOLARE (primo controllo)
   │  └─ Se T_pannelli > T_boiler + 5°C
   │     ├─ SI → Attiva POMPA SOLARE (circuito autonomo)
   │     │       └─ Panneli (SPCF 100L + Vitosol 300) riscaldano boiler
   │     │       └─ Gratis dal sole! ☀️
   │     │
   │     └─ NO → (sole insufficiente) Vai al passo 2
   │
   ├─ CONTROLLO POMPA DI CALORE (fallback)
   │  └─ Se sole NON basta (inverno/nuvoloso)
   │     ├─ Accende COMPRESSORE pompa di calore
   │     ├─ Porta acqua a 55°C
   │     └─ Consuma energia elettrica ⚡
   │
   └─ SE ENTRAMBI DISPONIBILI (molla/autunno)
      └─ INTEGRAZIONE IBRIDA:
         ├─ Sole porta boiler a 40°C (semi-riscaldato)
         ├─ Pompa completa da 40°C a 55°C
         └─ Meno lavoro per compressore = meno consumo! 🎯
```

### Scenario Reale: Estate vs Inverno

**ESTATE (Giugno-Agosto)**: ☀️ Sole Forte
```
9:00 AM - Settpoint 22°C impostato
     ↓
Vitotronic legge: T_boiler = 30°C (freddo, notte)
     ↓
Controlla T_pannelli solari: 75°C (molto caldo!)
     ↓
Errore solare: 75°C − 30°C = 45°C (ENORME!)
     ↓
AZIONE: Accende pompa solare autonoma
     ├─ Pannelli trasferiscono calore al boiler
     ├─ Boiler sale: 30°C → 40°C → 50°C → 60°C
     ├─ Raggiunge 60°C in ~30 minuti
     ├─ Pompa solare si spegne (T_boiler ≥ T_pannelli)
     │
     └─ RISULTATO: 
         ✅ ACS gratis dal sole (0 consumo elettrico!)
         ✅ Pompa di calore spenta tutto il giorno
         ✅ Risparmio: 0 kWh consumato
```

**INVERNO (Dicembre-Febbraio)**: ❄️ Sole Debole
```
9:00 AM - Setpoint 22°C impostato
     ↓
Vitotronic legge: T_boiler = 15°C (gelato, notte)
     ↓
Controlla T_pannelli solari: 25°C (appena più caldo)
     ↓
Errore solare: 25°C − 15°C = 10°C (insufficiente)
     ├─ Soglia minima richiesta: 5°C
     └─ CONDIZIONE SODDISFATTA, ma porta solo a 25°C
     ↓
AZIONE: Pompa solare accende (margine)
     ├─ Pannelli pre-riscaldano: 15°C → 20°C (~20 minuti)
     ├─ T_boiler sale a 20°C
     ├─ Pannelli non riescono a salire oltre
     └─ Pompa solare si spegne (differenza < 5°C)
     ↓
AZIONE: Accende POMPA DI CALORE (fallback)
     ├─ Compressore attiva al 100%
     ├─ Completa riscaldamento: 20°C → 55°C (~15 minuti)
     ├─ Boiler raggiunge target
     └─ Riscaldamento acceso alla casa (22°C mantenuto)
     │
     └─ RISULTATO:
         ⚡ Consumo: ~1,5-2 kWh (pompa + solare leggero)
         ✅ Ibrido: Sole fa 20%, pompa fa 80%
```

**MOLLA/AUTUNNO (Aprile-Maggio, Settembre-Ottobre)**: 🌤️ Sole Moderato
```
9:00 AM - Setpoint 22°C impostato
     ↓
Vitotronic legge: T_boiler = 25°C
     ↓
Controlla T_pannelli solari: 55°C (moderato)
     ↓
Errore solare: 55°C − 25°C = 30°C (buono!)
     ↓
AZIONE: Accende pompa solare (ha senso)
     ├─ Pannelli riscaldano: 25°C → 45°C (~45 min)
     ├─ Vitotronic vede T_boiler = 45°C
     └─ Differenza pannelli-boiler cala (55°C − 45°C = 10°C)
        → Pompa solare si spegne (margine ridotto)
     ↓
AZIONE: Accende pompa di calore (completamento)
     ├─ Compressore attiva al 40% (bassa modulazione)
     ├─ Completa: 45°C → 55°C rapidamente
     ├─ Minima potenza necessaria
     └─ Spegnimento immediato al raggiungimento
     │
     └─ RISULTATO:
         ⚡ Consumo: ~0,8-1 kWh (pompa modulata basso)
         ✅ Ibrido: Sole fa 50%, pompa fa 50%
         ✅ MASSIMA EFFICIENZA!
```

### Tabella Riassuntiva: Chi Riscalda Quando?

| Stagione | T_Pannelli | Sole Attivo? | Pompa Di Calore | Risultato |
|----------|-----------|------------|-----------------|-----------|
| **Giugno-Agosto** | 65-80°C | ✅ SI (100%) | ❌ Spenta | Sole 100% |
| **Maggio/Settembre** | 45-55°C | ✅ SI (parziale) | ✅ 30-50% | Ibrido 50-50% |
| **Aprile/Ottobre** | 35-45°C | ✅ SI (margine) | ✅ 60-80% | Sole 30% + Pompa 70% |
| **Novembre-Marzo** | 15-30°C | ⚠️ Margine | ✅ 90-100% | Pompa 80-100% |
| **Notte (sempre)** | 10-20°C | ❌ NO | ✅ 100% | Pompa 100% |

### Comandi Modbus per Monitoraggio

Se usi **ESP32**, puoi leggere:

```
REG 0  = T_mandata (temperatura uscita boiler) → 55°C
REG 2  = T_boiler (temperatura dentro boiler) → 45°C
REG 3  = T_esterna (temperatura aria esterna) → 5°C
REG 8  = Pompa solare attiva? → 0=NO, 1=SI

=== LOGICA VITOTRONIC INTERNA ===
Se (T_pannelli > T_boiler + 5°C) → Accendi pompa solare
Se (T_boiler < 55°C) → Accendi pompa di calore
Se (T_boiler ≥ 55°C) → Standby (protezione)
```

---

## Funzioni Principali

### 1. Buffer Termico
- Accumula calore dalla pompa di calore
- Livella picchi di carico termico
- Migliora efficienza sistema

### 2. Preparazione Acqua Calda Sanitaria (ACS)
- Sonda media mantiene 45-50°C per comfort
- Protezione 60°C contro batteri
- Collegamento caldaia ausiliaria per integrazione

### 3. Circuito Riscaldamento
- Distribuzione acqua riscaldata ai ventilconvettori
- Valvola miscelatrice 3-vie per modulazione temperatura
- Pompe circolazione giorno/notte

### 4. Integrazione Solare
- Pannello solare SPCF pre-riscalda ACS
- Riduce carico pompa di calore in estate

## Sonde Interne

| Sonda | Temperatura | Funzione | Modbus |
|-------|-------------|----------|--------|
| **Alta** | 60-75°C | Protezione boiler | REG 104 |
| **Media** | 40-55°C | Comfort ACS (45-50°C) | REG 106 |
| **Bassa** | 10-30°C | Protezione minima (<25°C allarme) | REG 107 |

## Schema Idraulico

```
POMPA CALORE (70°C)
       │ Mandata
       ▼
   ┌────────────────┐
   │   BOILER 97L   │
   │  VITOCAL 100-S │
   │                │
   │ ┌────────────┐ │
   │ │ Sonda Alta │ │
   │ │ (75°C max) │ │
   │ └────────────┘ │
   │                │
   │ ┌────────────┐ │
   │ │ Sonda Media│ │
   │ │ (45-50°C)  │ │
   │ └────────────┘ │
   │                │
   │ ┌────────────┐ │
   │ │Sonda Bassa │ │
   │ │(30°C min)  │ │
   │ └────────────┘ │
   │                │
   └────────────────┘
       │ Ritorno (45°C)
       ▼
   ┌─────┴──────────────┐
   │                    │
   ▼ ACS                ▼ Riscaldamento
(Caldaia +          (Ventilconvettori)
 Pannello Solare)
```

## Vantaggi

✅ Isolamento termico eccellente (zero dispersioni)
✅ 3 sonde per controllo preciso temperatura
✅ Protezione completa da corrosione (anodo magnesio)
✅ Capacità ideale per pompa 6kW
✅ Collegamento flessibile (ACS + riscaldamento + solare)
✅ Conformità alle normative europee
✅ Durabilità >20 anni

## Monitoraggio Allarmi

| Evento | Trigger | Azione |
|--------|---------|--------|
| Sonda guasta | No lettura | Alert HA + display |
| Pressione bassa | <1.5 bar | Allarme + spegnimento |
| Pressione alta | >3.5 bar | Scarico automatico |
| Temp alta | >75°C | Attivazione protezione |
| Temp bassa | <25°C | Avviso sensore |

## Manutenzione

- **Anodo magnesio**: Controllare ogni 2-3 anni (sostituire se consumato)
- **Spurgo aria**: Se pressione anomala
- **Isolamento**: Non danneggiare spessore 100mm
- **Pulizia**: Esterno con panno umido (mai spruzzare acqua sui raccordi)

## Documentazione Ufficiale

- [Viessmann Vitocal 100-S](https://www.viessmann.it)
- Manual installazione: Da richiedere a Viessmann
- Certificazione: CE, RoHS, Direttiva Macchine

---

*Fonte: Viessmann Italia - Prodotti Pompe di Calore*
*Aggiornamento: Aprile 2026*
