# Ventilconvettori (Fan-Coil) - Sistema Riscaldamento/Raffrescamento

## Panoramica

**2× Ventilconvettori 4-tubi** - Unità interne per distribuzione aria riscaldata/raffreddata negli ambienti. Controllabili tramite Modbus RTU per velocità ventola.

## Specifiche Tecniche

| Parametro | Valore |
|-----------|--------|
| **Tipo** | Fan-coil 4-tubi a cassetta |
| **Quantità** | 2 unità interne |
| **Configurazione** | Riscaldamento + Raffrescamento indipendente |
| **Portata aria** | Variabile (Auto/Min/Notte/Max) |
| **Velocità ventilatore** | 4 livelli (0=Auto, 1=Min, 2=Notte, 3=Max) |
| **Controllo velocità** | Modbus RTU (REG 101 bit 1-2) |
| **Temperatura ingresso** | Inv: 55°C, Est: 12-15°C |
| **Temperatura uscita** | Inv: 45°C, Est: 20°C |
| **Rumore** | Inv: ~30dB (min), ~45dB (max) |
|  | Est: ~25dB (min), ~40dB (max) |
| **Collegamento idraulico** | Valvola 3-vie per modulazione |
| **Filtro aria** | Estraibile, sostituibile |

## Ubicazione Unità

### Unità 1: SALOTTO
- Posizione: Nicchia parete
- Funzione: Riscaldamento principale + raffrescamento estivo
- Accesso: Facilmente visibile

### Unità 2: CAMERA DA LETTO
- Posizione: Secondo ambiente principale
- Funzione: Riscaldamento + raffrescamento secondario
- Accesso: Facilmente visibile

## Funzionamento RISCALDAMENTO (Inverno)

```
Boiler Vitocal 100-S (55°C)
       │ Mandata calda
       ▼
Valvola 3-vie (modulazione)
       │
       ├─→ FAN-COIL 1 (Salotto)
       │        │
       │        ├─→ Serpentino 4-tubi
       │        │        │ Assorbe aria fredda da ambiente
       │        │        ▼
       │        ├─→ Ventilatore (Auto/Min/Notte/Max)
       │        │        │ Soffia aria calda
       │        │        ▼
       │        └─→ Ritorno 45°C (boiler)
       │
       └─→ FAN-COIL 2 (Camera)
                   [stesso schema]
```

**Setpoint**: 20-22°C ambiente
**Fan speed**: Regolabile via Modbus (4 livelli)

## Funzionamento RAFFRESCAMENTO (Estate)

```
Pompa Calore (12-15°C acqua fredda)
       │ Mandata fredda
       ▼
Valvola 3-vie
       │
       ├─→ FAN-COIL 1
       │        │
       │        ├─→ Serpentino freddo (12-15°C)
       │        │        │ Assorbe calore da ambiente
       │        │        ▼
       │        ├─→ Ventilatore (Auto/Min/Max)
       │        │        │ Distribuisce aria fredda
       │        │        ▼
       │        ├─→ Condensa raccolto in vasca
       │        │        │ Drenaggio autonomo
       │        │        ▼
       │        └─→ Ritorno 20°C (pompa)
       │
       └─→ FAN-COIL 2
                   [stesso schema]
```

**Setpoint**: 26-28°C ambiente
**Condensa**: Vasca raccolta + drenaggio autonomo

## Valvola 3-Vie (Modulazione Temperatura)

```
Mandata pompa/boiler (55°C o 12°C)
       │
       ▼
   ┌───────────┐
   │ Valvola   │
   │  3-vie    │
   │ Motorizzata
   └───┬───┬───┘
       │   │
       │   └─→ Ritorno boiler (bypass se temp OK)
       │
       └─→ Fan-coil (modulazione portata)

Quando T ambiente = setpoint:
→ Valvola blocca mandata
→ Ricircola via bypass
→ Riduce carico pompa
```

## Controllo Velocità Ventola - Modbus

| Livello | Codice | Funzione | Rumore | Portata |
|---------|--------|----------|--------|---------|
| **Auto** | 0 | Modulazione automatica | Variabile | Variabile |
| **Min** | 1 | Minima velocità | ~25dB | ~30% |
| **Notte** | 2 | Ridotta (notturna) | ~20dB | ~20% |
| **Max** | 3 | Massima velocità | ~45dB | 100% |

**Invio comando Modbus**:
```
REG 101 bit 1-2 = velocità desiderata
Esempio: "00" = Auto, "11" = Max
```

## Manutenzione

### Filtri aria (ogni 1-2 mesi)
- Estraibile dalla griglia frontale
- Sostituzione con filtro G4 standard
- Se ostruito → riduce efficienza

### Pulizia serpentino (annuale)
- Aria compressa delicata (non acqua!)
- Polvere riduce scambio termico
- Attenzione: Non piegare alette

### Vasca condensa (estiva)
- Controllare drenaggio libero
- Pulizia con soluzione igienizzante
- Prevenire alghe/batteri in estate

### Isolamento tubi (annuale)
- Verificare integrità schiuma
- Se staccata → perdite termiche
- Tape adesivo isolante per riparazioni

## Regolazione Manuale (senza Modbus)

Se comunicazione Modbus non disponibile:
- **Valvola manuale**: Su raccordo "A" (modulazione %)
- **Velocità ventola**: Interruttore locale (se presente)
- **Bypass**: Valvola di sovrapressione per protezione

## Anomalie

| Sintomo | Causa | Soluzione |
|---------|-------|-----------|
| Nessun flusso aria | Filtro ostruito | Sostituire filtro G4 |
| Aria tiepida (riscald) | Valvola 3-vie bloccata | Pulire valve, disincrostare |
| Condensa abbondante | Serpentino troppo freddo | Regolare valvola bypass |
| Rumore eccessivo | Cavitazione/blocco | Purgar aria, controllare pressione |
| Vibrazione | Ventilatore sbilanciato | Contattare tecnico specializzato |

## Specifiche Costruttive

- **Materiale**: Acciaio/Alluminio verniciato
- **Isolamento carico**: Gommini antivibranti
- **Scarico condensa**: Tubo Ø16 mm con sifone
- **Attacchi idraulici**: Raccordi ottone 3/4" (20 mm)

---

*Fonte: Specifica progetto impianto Casa Molinella*
*Aggiornamento: Aprile 2026*
