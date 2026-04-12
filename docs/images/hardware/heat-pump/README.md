# Pompa di Calore Aria-Acqua Split - Vitocal 100-S

**Fonte:** Datasheet tecnico ufficiale Viessmann - Rev. 06/2018

## Panoramica

**Vitocal 100-S** - Pompa di calore splittata aria/acqua reversibile per riscaldamento, raffrescamento e acqua calda sanitaria. 

**IMPORTANTE**: La Vitocal 100-S **NON include boiler integrato** - deve essere collegata a un boiler/accumulo esterno (es. SPCF 100L).

*Nota: La Vitocal 111-S include invece boiler integrato 210L all-in-one*

Sistema split con unità esterna e interna collegata via gas refrigerante (evita congelamento tubazioni).

## Specifiche Tecniche

| Parametro | Valore |
|-----------|--------|
| **Potenza riscaldamento** | 3,2 - 17,1 kW (gamma completa) |
| **Potenza raffrescamento** | 1,5 - 17,0 kW (reversibile) |
| **Alimentazione** | Monofase 230V (mod. A04-A16) / Trifase 400V (mod. A12-A16) |
| **Temperatura mandata** | Fino a 55°C (con aria esterna -5°C) |
| **COP (A7/W35)** | 4,37 - 4,72 (riscaldamento) |
| **EER (A35/W18)** | 3,61 - 4,20 (raffrescamento) |
| **COP minimo (A-7/W35)** | 2,70 - 2,73 (temperature critiche) |
| **Classe energetica** | **A++** (per riscaldamento) |
| **Refrigerante** | R32 (basso GWP) - modelli 4-8 kW |
| **Range aria esterna** | -5°C a +35°C |
| **Configurazione** | **Split** (unità esterna + interna collegati via gas refrigerante) |
| **Comunicazione** | Modbus RTU (RS485) via Regolazione Vitotronic 200 |
| **Vaso espansione** | Integrato nell'unità interna |
| **Valvola sicurezza** | Integrata nell'unità interna |

## Componenti Unità Interna

```
UNITÀ INTERNA (Centrale idraulica - senza boiler)
┌─────────────────────────────────────┐
│ 1. Flussostato                      │ ← Verifica circolazione acqua
│ 2. Condensatore/Evaporatore         │ ← Scambio termico interno
│ 3. Pompa circolazione alta eff.     │ ← Movimenta acqua circuiti
│ 4. Valvola deviatrice riscald/ACS   │ ← Selezione circuito attivo
│ 5. Regolazione Vitotronic 200       │ ← Controllo sistema
│ • Vaso espansione (integrato)       │
│ • Valvola sicurezza (integrata)     │
│                                     │
│ ⚠️ BOILER ESTERNO RICHIESTO         │
│ (SPCF 100L o equivalente)           │
└─────────────────────────────────────┘
```

## Componenti Unità Esterna

```
UNITÀ ESTERNA (Pompa di calore aria-acqua)
┌─────────────────────────────────────┐
│ 1. Evaporatore                      │ ← Assorbe calore aria (-5°C a +35°C)
│ 2. Ventilatore                      │ ← Soffia aria sull'evaporatore
│ 3. Compressore                      │ ← Pompa refrigerante (cuore sistema)
│ • Gas refrigerante R32              │
└─────────────────────────────────────┘

Collegamento: Via tubo GAS REFRIGERANTE (non acqua)
↓
Vantaggi: Evita congelamento tubazioni in inverno
```

## Funzionamento

### Riscaldamento (Inverno, A7/W35)
```
Aria esterna (-5°C a +7°C)
       ↓
Evaporatore (unità esterna)
       ↓
Assorbe calore → Gas refrigerante pressurizzato
       ↓
Compressore (aumenta pressione e temperatura)
       ↓
Condensatore (unità interna)
       ↓
Riscalda acqua fino a 55°C
       ↓
Distribuisce ai ventilconvettori/radiatori
       ↓
COP 4,37-4,72 = 4,5 kW calore per 1 kW elettrico
```

**Modalità pompa notte**: Riduzione automatica carico compressore
**Modalità pompa giorno**: Massima modulazione inverter

### Raffrescamento (Estate, A35/W18)
```
Aria ambiente (26-28°C)
       ↓
Ventilconvettori (raffreddamento interno)
       ↓
Estrae calore ambiente → Acqua fredda a 7-18°C
       ↓
Compressore (ciclo inverso)
       ↓
Unità esterna (condensa calore all'esterno)
       ↓
EER 3,61-4,20 = 3,9 kW raffreddamento per 1 kW elettrico
```

**Protezione condensa**: Vasca raccolta con drenaggio autonomo nei fan-coil

## Modelli Disponibili

| Modello | Monofase | Potenza nom. (A7/W35) | Min-Max | Comp. |
|---------|----------|----------------------|---------|-------|
| **101.A04** | 230V | 4,54 kW | 3,2-8,4 | 1 |
| **101.A06** | 230V | 5,92 kW | 4,2-10,2 | 1 |
| **101.A08** | 230V | 8,20 kW | 5,2-12,1 | 1 |
| **101.A12** | 230V/400V | 11,50 kW | 6,1-13,0 | 1 |
| **101.A14** | 230V/400V | 13,50 kW | 7,0-15,0 | 1 |
| **101.A16** | 400V | 15,50-15,74 kW | 7,5-17,1 | 1 |

## Caratteristiche Principali

✅ **Sistema split** - Unità esterna + interna separate (rumore contenuto)
✅ **COP 4,37-4,72** - Efficienza riscaldamento classe A++
✅ **Tecnologia Inverter** - Modulazione continua potenza (non on/off)
✅ **Gas R32** - Ecologico e performante (modelli 4-8 kW)
✅ **Refrigerante via tubo** - Evita congelamento (connessione gas refrigerante)
✅ **Riscaldamento fino a -5°C** - Funziona con temperature critiche
✅ **Raffrescamento reversibile** - Stessa pompa per riscaldare e raffreddare
✅ **Regolazione Vitotronic 200** - Display grafico intuitivo
✅ **Modbus RTU** - Comunicazione con controller ESP32
✅ **Componenti integrati** - Vaso espansione e valvola sicurezza inclusi
✅ **Dimensioni compatte** - Unità interna 880×450×370 mm
✅ **Gestione App ViCare** - Controllo remoto via smartphone

## Dimensioni

### Unità Interna
- **Altezza**: 880 mm
- **Larghezza**: 450 mm
- **Profondità**: 370 mm
- **Facile integrazione** in mobile/armadio tecnico

### Unità Esterna
- Modelli A04-A08: **790×980×360 mm** (unità standard)
- Modelli A12-A16: **1345×900×412 mm** (unità grande per maggior potenza)

## Documentazione Ufficiale

- [Viessmann Vitocal 100-S - Prodotto](https://www.viessmann.it/it/prodotti/pompa-di-calore/vitocal-100-s.html)
- [Datasheet Tecnico Vitocal 100-S Rev. 06/2018](https://www.viessmann.it) - Viessmann Italia
- [Manuale Installazione e Servizio](https://www.viessmann.it/it/servizi-per-te/istruzioni-d-uso.html) - Da richiedere a Viessmann
- [App ViCare - Controllo remoto](https://www.viessmann.it/it/prodotti/controlli-e-connettivita/vitoconnect.html)
- [Interfaccia Vitoconnect per remote management](https://www.viessmann.it/it/prodotti/controlli-e-connettivita/vitoconnect.html)

---

*Fonte: Datasheet Tecnico Ufficiale Viessmann Vitocal 100-S Rev. 06/2018*
*Aggiornamento: Aprile 2026 (da fonti ufficiali Viessmann)*

