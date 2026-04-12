# Stazione di Pompaggio Solare - Viessmann Solar-Divicon PS10

**Fonte:** Viessmann - Documentazione tecnica prodotto

## Panoramica

**Solar-Divicon PS10** - Stazione di pompaggio compatta dedicata al circuito solare termico. Gestisce autonomamente la circolazione dell'acqua tra i pannelli solari (SPCF 100L + Vitosol 300-TM) e il boiler di accumulo (SPCF 100L).

**FUNZIONE CRITICA**: Senza questa pompa, i pannelli solari NON possono trasferire il calore al boiler. È il "cuore" del circuito solare.

## Componenti Principali

```
SOLAR-DIVICON PS10 (Stazione di pompaggio)
┌─────────────────────────────────────┐
│ 1. POMPA AD ALTA EFFICIENZA         │ ← Circola acqua tra pannelli e boiler
│    (velocità controllata)           │
│    Portata: 0-100 l/h              │
│    Prevalenza: 6,5 m @ 1000 l/h     │
│                                     │
│ 2. DUE TERMOMETRI                   │ ← Misurano T circuito solare
│    ├─ Manometro superiore           │   (pannelli vs boiler)
│    └─ Manometro inferiore           │   Lettura 0-100°C
│                                     │
│ 3. DUE VALVOLE A SFERA CON RITEGNO  │ ← Shutoff sicuro, non ritorno
│    ├─ Entrata da pannelli           │
│    └─ Uscita verso boiler           │
│                                     │
│ 4. INDICATORE DI FLUSSO              │ ← Visuale conferma circolazione
│    (pallina che sale quando pompa   │   pallina in su = OK ✅
│     circola acqua)                  │   pallina in giù = GUASTO ❌
│                                     │
│ 5. CONTROLLER SOLARE VITOSOLIC 100  │ ← Cervello pompaggio solare
│    Display digitale verde           │   Accende/spegne pompa
│    Sensori temperatura integrati    │   Protezioni automatiche
│                                     │
│ 6. CILINDRO GIALLO                  │ ← Vaso espansione circuito solare
│    (protezione pressione minima)    │   Assorbe dilatazione acqua
└─────────────────────────────────────┘
```

## Specifiche Tecniche

| Parametro | Valore |
|-----------|--------|
| **Modello** | Viessmann Solar-Divicon PS10 |
| **Tipo** | Stazione di pompaggio a due tubi |
| **Pompa** | Circolazione ad alta efficienza |
| **Portata max** | 1000 l/h (16,7 l/min) |
| **Prevalenza** | 6,5 m @ 1000 l/h |
| **Potenza motore** | ~25-40W (modulabile) |
| **Velocità** | Controllata da sensore differenziale |
| **Termometri** | 2 × 0-100°C (analogici) |
| **Pressione nominale** | 3 bar |
| **Pressione max** | 4 bar |
| **Valvole ritegno** | 2 × check valves (ritorno bloccato) |
| **Isolamento circuito** | Doppio tubo isolato termicamente |
| **Lunghezza max tubi** | ~30 metri da pannelli a boiler |
| **Fluido circuito** | Miscela glicole-acqua (protezione -20°C) |
| **Alimentazione** | 230V monofase AC, 50Hz |
| **Controller** | Vitosolic 100 SD1 (integrato) |

## Funzionamento

### Logica di Controllo (Vitosolic 100 integrato)

```
CICLO CONTINUO (controllo solare automatico):

1. LETTURA SENSORI
   ├─ T_pannelli (temperatura pannelli solari)
   ├─ T_boiler (temperatura boiler accumulo)
   └─ Differenza ΔT = T_pannelli - T_boiler

2. DECISIONE POMPA
   ├─ SE ΔT > 5°C (pannelli sufficientemente caldi)
   │  └─ ACCENDI pompa (modulazione graduale)
   │     └─ Acqua circola da pannelli → boiler
   │
   ├─ SE ΔT < 3°C (pannelli raffreddano)
   │  └─ SPEGNI pompa (protezione da perdita di calore)
   │     └─ Valvole ritegno bloccano ritorno
   │
   └─ SE 3°C < ΔT < 5°C (zona grigia)
      └─ Mantieni stato precedente (isteresi)

3. PROTEZIONI AUTOMATICHE
   ├─ Pressione > 4 bar → Scarico di sicurezza
   ├─ T_pannelli > 110°C → Blocco pompa (protezione surriscaldo)
   ├─ T_pannelli < -5°C → Anticongelamento attivo
   └─ Nessun flusso per 5 min → Allarme sensore

4. CONTROLLORE VITOTRONIC 200
   ├─ Legge stato Solar-Divicon via Modbus
   ├─ Se pompa attiva → Boiler in riscaldamento solare
   └─ Se pompa inattiva → Pompa di calore pronta
```

### Modalità Operativa Giornaliera

```
06:00 AM - Alba (sole debole)
└─ T_pannelli = 25°C, T_boiler = 20°C, ΔT = 5°C
   └─ SOGLIA RAGGIUNTA → Pompa accende lentamente
   └─ Riscaldamento solare inizia

12:00 PM - Picco solare
└─ T_pannelli = 85°C, T_boiler = 55°C, ΔT = 30°C
   └─ MASSIMO RENDIMENTO → Pompa 100%
   └─ Trasferisce 15-20 kW termici al boiler
   └─ Display Vitosolic mostra "SOL 100%"

16:00 PM - Sole cala
└─ T_pannelli = 45°C, T_boiler = 50°C, ΔT = -5°C
   └─ POMPA SPEGNE → Valvole ritegno proteggono
   └─ Evita perdita di calore da boiler ai pannelli

21:00 PM - Notte
└─ T_pannelli = 15°C (temperatura aria)
   └─ POMPA OFF → Sistema standby
   └─ Valvole bloccano completamente
   └─ Boiler mantiene calore isolato
```

## Schema Idraulico Completo

```
┌──────────────────────────────────┐
│  PANNELLI SOLARI                 │
│  SPCF 100L + Vitosol 300-TM      │
│  (T_pannelli 15-100°C)           │
└───────────────┬──────────────────┘
                │ Tubo mandata (80°C max)
                │ (isolato termicamente)
                ▼
    ┌───────────────────────────┐
    │ SOLAR-DIVICON PS10        │
    │                           │
    │ Pompa circolazione ← [O]  │ ← Motore 230V
    │ (1000 l/h max)           │
    │                           │
    │ T_PANNELLI ──┬─→ Display  │
    │              │ (lettura)  │
    │ Valvola ritegno (↑)       │
    │                           │
    │ T_BOILER ────┬─→ Display  │
    │              │ (lettura)  │
    │ Valvola ritegno (↓)       │
    │                           │
    │ Indicatore flusso (●●●●)  │ ← Pallina che sale/scende
    │                           │
    │ Vitosolic 100 SD1 ⭐      │ ← Controllo automatico
    └───────────────┬───────────┘
                    │ Tubo ritorno (40°C)
                    │ (isolato termicamente)
                    ▼
        ┌──────────────────────┐
        │  BOILER SPCF 100L    │
        │  97L accumulo        │
        │  ┌────────────────┐  │
        │  │ Sonda Media    │  │
        │  │ (45-50°C ACS)  │  │
        │  └────────────────┘  │
        │                      │
        │  ┌────────────────┐  │
        │  │ Sonda Alta     │  │
        │  │ (60°C max)     │  │
        │  └────────────────┘  │
        └──────────────────────┘
               │
         ┌─────┴─────────────┐
         │                   │
         ▼ ACS (45-50°C)    ▼ Riscaldamento (55°C)
    [Rubinetti]      [Ventilconvettori]
```

## Manutenzione

### Mensile
- ✅ Verificare pallina indicatore flusso
  - IN SU (●●●●) = Pompa circola, OK ✅
  - IN GIÙ (○○○○) = Pompa guasta, ALLARME ❌
- ✅ Leggere temperatura termometri
  - Differenza T_pannelli - T_boiler dovrebbe essere 5-30°C durante il giorno

### Annuale
- 🔄 Verificare isolamento tubi (nessuna condensazione)
- 🔄 Controllare connessioni valvole (assenza perdite)
- 🔄 Ispezionare cilindro giallo (vaso espansione intatto)
- 🔄 Verificare pressione sistema (1,5-2,0 bar con pompa spenta)
- 🔄 Pulizia sensori temperatura (se visibili)

### Se Allarme o Malfunzionamento

| Sintomo | Causa Probabile | Soluzione |
|---------|---|---|
| Pallina indicatore ferma in giù | Pompa guasta o bloccata | Contattare tecnico Viessmann |
| Pressione > 4 bar | Dilatazione fluido solare | Spurgo automatico, verificare vaso |
| T_pannelli non sale | Pannelli sporchi/ghiaccio | Pulire con acqua demineralizzata |
| T_boiler non aumenta | Pompa inattiva (normale inverno) | Se estate, controllare sensori |
| Perdita fluido | Valvola rotta | Contattare tecnico |

## Integrazione con Vitotronic 200 (Vitocal 100-S)

Il Solar-Divicon PS10 **comunica autonomamente** ma il Vitotronic 200 **monitora lo stato**:

```
SOLAR-DIVICON PS10
      ↓ (stato pompa)
Modbus RTU (RS485)
      ↓
VITOTRONIC 200
      │
      ├─ Se pompa solare ATTIVA
      │  └─ Riduce compressore (pompa calore modulata bassa)
      │  └─ Risparmia energia
      │
      └─ Se pompa solare INATTIVA
         └─ Compressore pronto per riscaldamento
         └─ Modalità standby efficiente
```

### Comandi Modbus Lettura

```
REG 8 = Pompa solare attiva?
        0 = NO (spenta)
        1 = SI (circolazione attiva)

REG 3 = T_esterna (da sensore solare)
REG 4 = T_boiler (feedback circuito)
```

## Specifiche Fluido Circuito Solare

**Tipo**: Miscela glicole-acqua (50% glicole - 50% acqua demineralizzata)

**Proprietà**:
- ✅ Protezione antigelo fino a -20°C
- ✅ Protezione dall'ebollizione fino a 110°C
- ✅ Conduttività termica ottimale
- ✅ Compatibile con tutti i materiali circuito

**Volume circuito solare**: ~5-8 litri (a seconda lunghezza tubi)

**Riempimento**: Ogni 3-5 anni o se perdite

## Posizionamento nel Sistema Casa Molinella

```
CIRCUITO SOLARE AUTONOMO (indipendente)

Pannelli Solari
    ↓
SOLAR-DIVICON PS10 (pompa + controllo)
    ↓
Boiler SPCF 100L (pre-riscaldamento)
    ↓
├─ ACS (45-50°C comfort)
│  └─ Rubinetti / Servizi
│
└─ Riscaldamento (55°C)
   ├─ Ventilconvettore #1
   └─ Ventilconvettore #2
        ↓ (se estate o freddo)
   POMPA DI CALORE Vitocal 100-S (integrazione)
```

---

*Fonte: Viessmann - Documentazione Tecnica Solar-Divicon PS10*  
*Modello: Z021902 (con Vitosolic 100 SD1)*  
*Aggiornamento: Aprile 2026*
