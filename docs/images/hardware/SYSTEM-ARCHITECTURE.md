# Architettura Sistema Viessmann Casa Molinella - 2 Circuiti Indipendenti

**Fonte:** Documentazione Viessmann + Configurazione Casa Molinella

## Panoramica

Sistema di riscaldamento completo Viessmann con **2 CIRCUITI IDRAULICI SEPARATI E INDIPENDENTI**:

1. **CIRCUITO RISCALDAMENTO/RAFFREDDAMENTO** (Vitocal 100-S + Boiler SPCF 100L)
2. **CIRCUITO ACS** (Vitocal 060-A con serbatoio integrato)

Entrambi possono ricevere calore dai **pannelli solari** (circuito solare autonomo).

---

## Schema Completo Sistema

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    CASA MOLINELLA - SISTEMA VIESSMANN                       │
│                          2 CIRCUITI SEPARATI                                │
└─────────────────────────────────────────────────────────────────────────────┘

╔════════════════════════════════════════════════════════════════════════════════╗
║                        CIRCUITO 1: RISCALDAMENTO/RAFFREDDAMENTO               ║
╚════════════════════════════════════════════════════════════════════════════════╝

    PANNELLI SOLARI (SPCF 100L + Vitosol 300-TM)
           ↓ (65-85°C estate, 15-30°C inverno)
    ┌──────────────────────────┐
    │  SOLAR-DIVICON PS10      │ ← Pompa solare autonoma
    │  (se T_pannelli > T_boiler + 5°C)
    └────────────┬─────────────┘
                 │ (40°C ritorno)
                 ▼
    ╔════════════════════════════════════════════╗
    ║  BOILER SPCF 100L (97L ESTERNO)           ║
    ║  ┌────────────────────────────────────┐   ║
    ║  │ SONDA ALTA (60-75°C)               │   ║
    ║  │ Riscaldamento circuito principale  │   ║
    ║  └────────────────────────────────────┘   ║
    ║  ┌────────────────────────────────────┐   ║
    ║  │ SONDA BASSA (10-30°C)              │   ║
    ║  │ Raffreddamento circuito principale │   ║
    ║  └────────────────────────────────────┘   ║
    ╚────┬─────────────────────────────────────┬─╝
         │                                     │
    ┌────▼──────────────────────────┐   ┌────▼──────────────────┐
    │                               │   │                       │
    │ 55°C (Riscaldamento)          │   │ 12-15°C (Estate)      │
    │ Modalità POMPA GIORNO/NOTTE   │   │ Raffreddamento        │
    │                               │   │                       │
    ▼                               ▼   ▼                       ▼
    
  VENTILCONVETTORE #1        VENTILCONVETTORE #2
  ┌──────────────────┐       ┌──────────────────┐
  │ 4 tubi           │       │ 4 tubi           │
  │ Valvola 3-vie    │       │ Valvola 3-vie    │
  │ Modbus RTU       │       │ Modbus RTU       │
  │                  │       │                  │
  │ Modula:          │       │ Modula:          │
  │ • Mandata temp   │       │ • Mandata temp   │
  │ • Velocità fan   │       │ • Velocità fan   │
  │ • Modalità       │       │ • Modalità       │
  └──────────────────┘       └──────────────────┘
        │                           │
        ▼                           ▼
    AMBIENTE INTERNO
    Mantiene 22°C (setpoint)


    POMPA DI CALORE - VITOCAL 100-S
    ┌────────────────────────────────────────┐
    │ UNITÀ ESTERNA                          │
    │ (giardino, terrazza)                   │
    │ ├─ Evaporatore (assorbe calore aria)   │
    │ ├─ Ventilatore                         │
    │ └─ Compressore (inverter)              │
    │    Range: -5°C a +35°C aria esterna    │
    └────────┬─────────────────────────────┬─┘
             │ Gas refrigerante (tubo)     │
             │ (evita congelamento)        │
    ┌────────▼────────────────────────────▼─┐
    │ UNITÀ INTERNA (centralina idraulica)   │
    │ ├─ Flussostato                         │
    │ ├─ Condensatore/Evaporatore interno    │
    │ ├─ Pompa circolazione (alta eff.)      │
    │ ├─ Valvola deviatrice 3-vie            │
    │ └─ VITOTRONIC 200 ⭐                   │
    │    • Display grafico color             │
    │    • Modbus RTU (RS485)                │
    │    • Controllo automatico              │
    │    • Protezioni (antigelo, max temp)   │
    └────────┬────────────────────────────┬─┘
             │                            │
             ▼ Mandata 55°C               ▼ Ritorno 45°C
             │                            │
             └────────────────────────────┘
                      │
                      ▼
              BOILER SPCF 100L
              (accumula calore)


═══════════════════════════════════════════════════════════════════════════════════


╔════════════════════════════════════════════════════════════════════════════════╗
║              CIRCUITO 2: ACS (ACQUA CALDA SANITARIA) - SEPARATO               ║
╚════════════════════════════════════════════════════════════════════════════════╝

    PANNELLI SOLARI (SPCF 100L + Vitosol 300-TM)
           ↓ (65-85°C estate, 15-30°C inverno)
    ┌──────────────────────────┐
    │  SOLAR-DIVICON PS10      │ ← Pompa solare autonoma
    │  (se T_pannelli > T_boiler + 5°C)
    └────────────┬─────────────┘
                 │ (40°C ritorno - opzionale per 060-A)
                 │
                 ▼ (se ha scambiatore solare integrato)
    
    ╔════════════════════════════════════════════╗
    ║  VITOCAL 060-A (MONOBLOCCO COMPATTO)       ║
    ║  ┌────────────────────────────────────┐   ║
    ║  │ POMPA DI CALORE INTEGRATA          │   ║
    ║  │ (solo per ACS, non per riscaldam)  │   ║
    ║  └────────────────────────────────────┘   ║
    ║  ┌────────────────────────────────────┐   ║
    ║  │ SERBATOIO INTEGRATO INTERNO        │   ║
    ║  │ 178-254 LITRI (NON ESTERNO!)       │   ║
    ║  │                                    │   ║
    ║  │ ⭐ Tutto integrato in una unità!   │   ║
    ║  │                                    │   ║
    ║  │ • Pompa di calore riscalda acq.   │   ║
    ║  │ • Serbatoio accumula ACS          │   ║
    ║  │ • Scambiatore solare (opzionale)  │   ║
    ║  │ • Protezione 60°C (batteri)       │   ║
    ║  │ • COP 3.1 @ A2/W40                │   ║
    ║  └────────────────────────────────────┘   ║
    ║                                            ║
    ║  Modalità operativa:                       ║
    ║  ├─ Se T_acqua < 45°C → Pompa accende    ║
    ║  ├─ Riscalda fino a 50°C (comfort)       ║
    ║  ├─ Se > 60°C → Protezione termostatica  ║
    ║  └─ Effettua antilegionella (65°C) ogni  ║
    ║     7 giorni automaticamente              ║
    ╚────────────┬─────────────────────────────╝
                 │
                 ▼ 45-50°C (ACS riscaldata)
                 │
    ┌────────────┴────────────┐
    │                         │
    ▼                         ▼
  
  RUBINETTI        LAVATRICE/LAVASTOVIGLIE
  ├─ Doccia 45-50°C    └─ Acqua tiepida 45°C
  ├─ Lavandino
  └─ Vasca


═══════════════════════════════════════════════════════════════════════════════════


╔════════════════════════════════════════════════════════════════════════════════╗
║                    CIRCUITO SOLARE AUTONOMO (INDIPENDENTE)                     ║
╚════════════════════════════════════════════════════════════════════════════════╝

    PANNELLI SOLARI TERMICI
    ├─ SPCF 100L (collettore piano)      65-75% efficienza
    └─ Vitosol 300-TM (sottovuoto)       80-85% efficienza
    
    Stagionalità:
    ├─ Estate (giugno-agosto): 100% ACS dal sole ☀️ (pompa calore spenta)
    ├─ Molla/Autunno: 50-80% ACS dal sole + pompa completa
    └─ Inverno (nov-marzo): 10-30% solare + 70-90% pompa calore

    ┌──────────────────────────────────────────────┐
    │  SOLAR-DIVICON PS10                          │
    │  (Stazione di pompaggio solare autonoma)     │
    │                                              │
    │  Logica di controllo automatica:             │
    │  IF (T_pannelli > T_boiler + 5°C)           │
    │    THEN Accendi pompa solare                │
    │  ELSE Spegni pompa solare                   │
    │                                              │
    │  Protegge da:                               │
    │  • Surriscaldo pannelli (110°C max)         │
    │  • Congelamento (-20°C protezione)          │
    │  • Perdite pressione (valvole ritegno)      │
    └────────┬──────────────┬──────────────────┬──┘
             │              │                  │
             ▼              ▼                  ▼
      Boiler SPCF     Vitocal 060-A        Circuito
      (riscaldamento) (se presente)        autonomo
```

---

## Tabella Comparativa: Circuito 1 vs Circuito 2

| Aspetto | CIRCUITO 1 (Riscaldamento/Raffreddamento) | CIRCUITO 2 (ACS) |
|--------|---|---|
| **Pompa di calore** | Vitocal 100-S (split, 8.2-15.5 kW) | Vitocal 060-A (monoblocco, 1.3 kW) |
| **Serbatoio accumulo** | SPCF 100L esterno (97L) | Integrato in 060-A (178-254L) |
| **Temperatura** | 55°C (riscaldam) / 12-15°C (raffresc) | 45-50°C comfort / 60°C protezione |
| **Distribuzione** | Ventilconvettori (2×) | Rubinetti, lavatrice, lavastoviglie |
| **Controllo** | Vitotronic 200 (intelligente) | Termostato integrato (semplice) |
| **COP/EER** | 4.37-4.72 (riscaldam) / 3.61-4.20 (raffresc) | 3.1 @ A2/W40 |
| **Integrazione solare** | Riceve da Solar-Divicon PS10 | Riceve da Solar-Divicon PS10 (se scambiatore) |
| **Efficienza** | Molto alta (accumulo, modulazione) | Alta (serbatoio integrato) |
| **Rumore** | Discreto (unità esterna 42-50 dB) | Basso (integrata in casa) |

---

## Flusso Completo Durante una Giornata Tipo (Aprile)

### MATTINA (6:00-9:00 AM) - Freddo, Riscaldamento Attivo

```
☀️ Sole debole (T_pannelli = 30°C)
T_boiler = 20°C (freddo, notte)

CIRCUITO 1 (Riscaldamento):
  Vitotronic legge: T_ambiente = 18°C < setpoint 22°C
    → Accende pompa di calore 100%
    → Boiler sale: 20°C → 55°C (~20 min)
    → Esce da sonda alta: 55°C → Ventilconvettori
    → Fan-coil riscaldano: 18°C → 22°C

CIRCUITO 2 (ACS):
  Vitocal 060-A legge: T_serbatoio = 35°C < 45°C
    → Accende pompa autonomamente
    → Riscalda: 35°C → 50°C (~15 min)
    → Pronto per doccia mattutina ✅

CIRCUITO SOLARE:
  Solar-Divicon: T_pannelli (30°C) - T_boiler (55°C) = -25°C
    → Pompa solare SPENTA (differenza negativa)
    → Valvole ritegno proteggono da perdite ❌
```

### MEZZOGIORNO (12:00-14:00 PM) - Sole Forte

```
☀️☀️ Sole forte (T_pannelli = 80°C)
T_boiler = 55°C (riscaldato)

CIRCUITO 1 (Riscaldamento):
  Vitotronic legge: T_ambiente = 22°C = setpoint 22°C
    → Pompa di calore SPENTA (target raggiunto)
    → Boiler mantiene 55°C dai pannelli solari
    → Ventilconvettori modulati basso (marcia dolce)

CIRCUITO 2 (ACS):
  Vitocal 060-A legge: T_serbatoio = 52°C > 50°C
    → Pompa autonoma SPENTA (raggiunto comfort)
    → Serbatoio mantiene 50-52°C
    → Pronto per doccia istantanea ✅

CIRCUITO SOLARE:
  Solar-Divicon: T_pannelli (80°C) - T_boiler (55°C) = 25°C (GRANDE!)
    → Pompa solare ACCESA al 100%
    → Trasferisce calore pannelli → Boiler
    → Nessun consumo elettrico dalla pompa di calore! 🌞
    
RISULTATO: Impianto quasi completamente alimentato dal sole!
Consumo energetico quasi ZERO per riscaldamento/ACS
```

### POMERIGGIO (16:00-18:00 PM) - Sole Cala, Raffreddamento Attivo

```
🌤️ Sole moderato, Estate inizia (T_pannelli = 50°C)
T_esterno = 28°C (caldo)

CIRCUITO 1 (Raffreddamento):
  Vitotronic legge: T_ambiente = 26°C > setpoint 26°C (modalità estate)
    → Accende pompa di calore in ciclo INVERSO
    → Produce acqua fredda 12-15°C
    → Esce da sonda bassa: 12°C → Ventilconvettori
    → Fan-coil raffreddano: 26°C → 26°C (comfort)

CIRCUITO 2 (ACS):
  Vitocal 060-A legge: T_serbatoio = 48°C (calo lento dal pomeriggio)
    → Modalità SOFT (bassa potenza)
    → Mantiene 48-50°C senza sprecare energia
    → Pronto per doccia serale ✅

CIRCUITO SOLARE:
  Solar-Divicon: T_pannelli (50°C) - T_boiler (55°C) = -5°C
    → Pompa solare SPENTA (pannelli diventano freddi)
    → Valvole ritegno impediscono perdite
```

### SERA/NOTTE (20:00-22:00 PM) - Raffreddamento Modulato

```
🌙 Notte, temperatura cala (T_esterno = 18°C)

CIRCUITO 1 (Modalità Notte):
  Vitotronic switch a "Pompa Notte"
    → Riduce compressore a 30-50%
    → Mantiene T_ambiente = 20°C (setpoint notturno)
    → Ventilconvettori a velocità minima
    → Silenzioso, efficiente

CIRCUITO 2 (ACS):
  Vitocal 060-A:
    → Modalità standby termico
    → Mantiene serbatoio a 48°C (protezione energia)
    → Pronto per doccia mattutina ✅

CIRCUITO SOLARE:
  Solar-Divicon:
    → Completamente OFF
    → Pannelli raffreddati a T_esterna (18°C)
    → Circuito in standby

RISULTATO: Sistema in modalità efficiente notturna
```

---

## Protezioni e Logica di Controllo

### VITOTRONIC 200 (Circuito 1)

```
LETTURE CONTINUE:
├─ T_ambiente (da sensore stanza)
├─ T_boiler (sonda interna)
├─ T_esterna (da sensore esterno)
├─ Pressione circuito
└─ Stato ventilconvettori

DECISIONI AUTOMATICHE:
├─ IF T_ambiente < setpoint → Riscaldamento
├─ IF T_ambiente > setpoint → Raffreddamento
├─ IF T_boiler > 75°C → Protezione surriscaldo
├─ IF T_esterna < -5°C → Protezione antigelo
└─ IF ora_notturna → Riduz. potenza (modalità notte)

MODULAZIONE INVERTER:
├─ Compressore: 0-100% gradualmente (non on/off)
├─ Pompa circolazione: 0-100% dinamicamente
└─ Ventilconvettori: velocità 0-3 (automatica via fan-coil)
```

### VITOCAL 060-A (Circuito 2)

```
LETTURE:
├─ T_serbatoio interno (termostato integrato)
└─ T_ambiente (air intake da esterno o interno)

DECISIONI AUTOMATICHE:
├─ IF T_serbatoio < 45°C → Pompa di calore accende
├─ IF T_serbatoio ≥ 50°C → Pompa di calore spegne
├─ IF T_serbatoio > 60°C → Protezione termostatica (scarico)
└─ Ogni 7 giorni: Antilegionella (riscalda 65°C automaticamente)

OPERAZIONE:
└─ Completamente autonoma (non ha modbus integrato)
```

### SOLAR-DIVICON PS10 (Circuito Solare)

```
LETTURE:
├─ T_pannelli (da sensore pannelli)
└─ T_boiler (da sensore boiler circuito solare)

DECISIONE:
├─ IF (T_pannelli - T_boiler) > 5°C → Pompa accende
├─ IF (T_pannelli - T_boiler) < 3°C → Pompa spegne
└─ Isteresi 2°C per evitare on/off continui

PROTEZIONI:
├─ IF T_pannelli > 110°C → Blocco pompa (surriscaldo)
├─ IF T_pannelli < -5°C → Antigelo automatico (glycol)
└─ Flussostato monitora assenza circolazione
```

---

## Integrazione Modbus RTU (ESP32 Letture)

```
VITOTRONIC 200 (RS485 9600 baud):
├─ REG 0: T_mandata (riscaldamento)
├─ REG 2: T_boiler
├─ REG 3: T_esterna
├─ REG 4: T_ACS (se presente)
├─ REG 5: Potenza compressore (%)
├─ REG 6: Modalità operativa (1=riscald, 2=notte, 3=raffresc)
├─ REG 8: Pompa solare attiva (0/1)
└─ REG 100: Codice allarme

VITOCAL 060-A:
└─ NON ha Modbus integrato (completamente autonomo)

SOLAR-DIVICON PS10:
└─ NON ha Modbus integrato (completamente autonomo)
```

---

## Flusso Energetico: Dove Viene il Calore?

```
ESTATE (Massima Efficienza):
Sole → Pannelli solari → Solar-Divicon PS10 → Boiler/060-A
Consumo energetico: MINIMO (solo pompe) ⭐⭐⭐

PRIMAVERA/AUTUNNO (Ibrido):
Sole 50% + Pompa calore 50%
Pannelli → Solar-Divicon → Boiler (pre-riscalda)
Pompa calore completa il riscaldamento (lavoro ridotto)
Consumo: MODERATO (efficiente) ⭐⭐

INVERNO (Riscaldamento Attivo):
Pompa calore 80% + Sole 20% (margine)
Pannelli → Piccolo contributo
Pompa calore fa tutto il lavoro
Consumo: ELEVATO ma con COP 4.5 (4.5 kW calore per 1 kW elettrico)
```

---

## Documentazione dei Componenti

### Circuito 1: Riscaldamento/Raffreddamento
- 📄 [heat-pump/README.md](heat-pump/README.md) - Vitocal 100-S
- 📄 [heat-pump/unita-esterna/README.md](heat-pump/unita-esterna/README.md)
- 📄 [heat-pump/unita-interna/README.md](heat-pump/unita-interna/README.md) - Vitotronic 200
- 📄 [boiler/README.md](boiler/README.md) - SPCF 100L (97L)
- 📄 [fan-coil/README.md](fan-coil/README.md) - Ventilconvettori

### Circuito 2: ACS
- 📄 [scaldacqua/README.md](scaldacqua/README.md) - Vitocal 060-A

### Circuito Solare (Autonomo)
- 📄 [solar/README.md](solar/README.md) - Pannelli (SPCF 100L + Vitosol 300-TM)
- 📄 [solar-divicon-ps10/README.md](solar-divicon-ps10/README.md) - Pompa solare

### Controllo e Distribuzione
- 📄 [controller/README.md](controller/README.md) - Quadro interruttori

---

*Fonte: Documentazione Viessmann + Configurazione Casa Molinella*  
*Architettura: 2 Circuiti Indipendenti (Riscaldamento/Raffreddamento + ACS)*  
*Aggiornamento: Aprile 2026*
