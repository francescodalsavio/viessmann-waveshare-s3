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
