# Vitocal 100-S - Unità Interna (Centralina Idraulica)

**Fonte:** Datasheet tecnico ufficiale Viessmann - Rev. 06/2018

## Panoramica

**Unità interna della Vitocal 100-S** - Centralina idraulica che gestisce tutti i circuiti di riscaldamento, raffrescamento e acqua calda sanitaria. Contiene il **Vitotronic 200** (cervello del sistema).

**IMPORTANTE**: L'unità interna NON include boiler integrato - deve essere collegata a un boiler esterno (es. SPCF 100L).

## Componenti Unità Interna

```
UNITÀ INTERNA VITOCAL 100-S (Centralina Idraulica)
┌────────────────────────────────────────┐
│ 1. FLUSSOSTATO                         │ ← Verifica circolazione acqua
│                                        │
│ 2. CONDENSATORE/EVAPORATORE            │ ← Scambio termico interno
│    (serpentino interno)                │
│                                        │
│ 3. POMPA CIRCOLAZIONE ALTA EFFICIENZA  │ ← Movimenta acqua nei circuiti
│                                        │
│ 4. VALVOLA DEVIATRICE 3-VIE            │ ← Selezione circuito attivo:
│                                        │   • Riscaldamento
│                                        │   • Raffrescamento
│                                        │   • ACS (acqua calda sanitaria)
│                                        │
│ 5. REGOLAZIONE VITOTRONIC 200 ⭐       │ ← CONTROLLO SISTEMA (il cervello!)
│    • Display grafico color             │   • Modbus RTU (RS485)
│    • Letture temperatura               │   • App ViCare (remoto)
│    • Programmi orari                   │   • Regole riscaldamento/ACS
│                                        │
│ COMPONENTI INTEGRATI:                  │
│ • Vaso espansione (protezione p. min)  │
│ • Valvola sicurezza 3 bar              │
│ • Flussostato di protezione            │
└────────────────────────────────────────┘
```

## Funzionamento

### Circuiti Gestiti

```
UNITÀ INTERNA (Centralina Idraulica)
       ↓
Gestisce 3 circuiti indipendenti:

1️⃣ CIRCUITO RISCALDAMENTO (Inverno)
   Boiler esterno (55°C) → Valvola modulante → Fan-coil/Radiatori
   Setpoint: 20-22°C ambiente
   
2️⃣ CIRCUITO RAFFRESCAMENTO (Estate)
   Pompa calore (12-15°C) → Valvola modulante → Fan-coil
   Setpoint: 26-28°C ambiente
   Condensa: Vasca raccolta nei fan-coil
   
3️⃣ CIRCUITO ACS (Acqua Calda Sanitaria)
   Pompa calore integrata in boiler esterno
   Temperatura: 45-50°C comfort (60°C protezione)
   Integrazione solare (pre-riscaldamento)
```

### Modalità Operativa

| Modalità | Funzione | Compressore | Valvola |
|----------|----------|-------------|---------|
| **Pompa Giorno** | Riscaldamento completo | 100% | Riscald. |
| **Pompa Notte** | Modulazione ridotta | 30-70% | Riscald. |
| **Estate** | Raffrescamento | 100% | Raffres. |
| **ACS** | Acqua calda | 100% | ACS |
| **Solare** | Se T_solare > T_boiler | Spento | Bypass |

## Vitotronic 200 - Il Cervello del Sistema

### Display Principale
```
┌─────────────────────────────┐
│  VITOTRONIC 200 (Display)   │
│                             │
│ • Temperatura attuale       │
│ • Temperatura setpoint      │
│ • Modalità operativa        │
│ • Stato pompa               │
│ • Errori/Allarmi            │
│ • Lettura sensori           │
│                             │
│ Comunicazione Modbus RTU    │
│ (RS485 a 9600 baud)         │
└─────────────────────────────┘
```

### Funzioni Vitotronic 200
✅ Regolazione temperatura automatica
✅ Programmazione oraria (pompa giorno/notte)
✅ Protezione antigelo (-5°C)
✅ Protezione surriscaldamento (60°C)
✅ Monitoraggio sensori temperatura
✅ Gestione integrazione solare
✅ Comunicazione Modbus RTU (ESP32)
✅ App ViCare (remoto via Vitoconnect)

## Specifiche Tecniche Unità Interna

| Parametro | Valore |
|-----------|--------|
| **Dimensioni** | 880 × 450 × 370 mm (compatta) |
| **Vaso espansione** | Integrato (protezione pressione) |
| **Valvola sicurezza** | 3 bar integrata |
| **Flussostato** | Protezione circolazione minima |
| **Pompa circolazione** | Alta efficienza, modulante |
| **Condensatore interno** | Scambio termico gas/acqua |
| **Collegamento gas** | Tubo refrigerante da unità esterna |
| **Collegamento idraulico** | Attacchi 1" per mandata/ritorno |
| **Alimentazione** | 230V monofase o 400V trifase |
| **Potenza assorbimento** | 0,5-2,5 kW (variabile) |

## Collegamenti Unità Interna

```
COLLEGAMENTI FISICI:

Dalla pompa di calore (unità esterna):
├─ Tubo gas refrigerante IN (pressione) → Condensatore
└─ Tubo gas refrigerante OUT (ritorno) ← Condensatore

Alla distribuzione:
├─ Mandata riscaldamento (55°C) → Boiler esterno
├─ Ritorno riscaldamento (45°C) ← Boiler esterno
├─ Mandata ACS → Caldaia/Serbatoio
└─ Ritorno ACS ← Caldaia/Serbatoio

Controllo:
├─ Vitotronic 200 Display (interfaccia utente)
├─ Modbus RTU (RS485) ← ESP32 Controller
└─ App ViCare (remoto via Vitoconnect)
```

## Manutenzione

### Mensile
- ✅ Verificare letture Vitotronic 200
- ✅ Controllo assenza perdite

### Annuale
- 🔄 Spurgo aria circuiti (se pressione anomala)
- 🔄 Verifica isolamento tubi
- 🔄 Controllo integrità vaso espansione
- 🔄 Pulizia sensori temperatura

### Se Allarme Vitotronic 200
- Leggere codice errore su display
- Verificare comunicazione Modbus
- Controllare sensori temperatura
- Contattare tecnico se persistente

## Regolazione Vitotronic 200 - Comandi Modbus

```
REG 101 = Velocità ventola (fan-coil)
REG 104 = Sonda temperatura alta
REG 106 = Sonda temperatura media (ACS)
REG 107 = Sonda temperatura bassa

Comunicazione via RS485 (9600 baud) dall'ESP32
```

---

*Fonte: Datasheet Viessmann Vitocal 100-S Rev. 06/2018*
*Aggiornamento: Aprile 2026*
