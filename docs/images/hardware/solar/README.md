# Pannello Solare Termico - Viessmann Solarcell SPCF 100L

## Panoramica

**Viessmann Solarcell SPCF 100L** - Pannello solare termico per pre-riscaldamento acqua calda sanitaria (ACS). Riduce carico pompa di calore in stagione estiva.

## Specifiche Tecniche

| Parametro | Valore |
|-----------|--------|
| **Modello** | Viessmann Solarcell SPCF 100L |
| **Tipo** | Collettore piano selettivo |
| **Potenza certificata** | 35W (EN 12975) |
| **Superficie utile** | ~1.2 m² (stima) |
| **Volume fluido** | ~1.5 litri |
| **Pressione nominale** | 3 bar |
| **Materiale assorbitore** | Rame selettivo (alto assorbimento) |
| **Copertura** | Vetro temperato basso-ferro |
| **Isolamento retro** | Lana minerale 50mm |
| **Collegamento** | 3/4" (20mm) rame |
| **Fluido termovettore** | Acqua-glicole (anticongelante) |
| **Montaggio** | Tetto/Terrazza inclinata 30-45° |

## Funzionamento (Estate)

```
SOLARE TERMICO (sunny day 800 W/m²)
       │
       ├─ Radiazione solare → Assorbitore rame
       │  
       ├─ Calore assorbitore: 60-80°C
       │  
       ├─ Pompa solare attivata (Δt > 5°C)
       │
       ▼
   Circuito autonomo:
   Collettore (80°C) → Scambiatore boiler
                          │
                          ├─ Pre-riscalda acqua fredda
                          │
                          ├─ Riduce carico pompa calore
                          │
                          └─ Ritorno collettore (40°C)
       │
       ▼
   Boiler Vitocal 100-S
   └─ ACS pre-riscaldata da solare
      (Pompa calore integra solo ΔT rimanente)
```

## Vantaggi Estivi

✅ **Riduce consumo pompa calore** - Pre-riscalda a 60°C (pompa integra solo ultimi °C)
✅ **COP + elevato** - Pompa lavora meno (delta termico minore)
✅ **Acqua calda gratuita** - Energia del sole, zero consumo elettrico
✅ **Spegnimento pompapompa possibile** - Se solare copre 100% fabbisogno ACS
✅ **Risparmio estivo significativo** - 30-50% consumi ACS

## Protezione Invernale

In inverno il pannello solare è:
- **Sospeso** (pompa solare non attiva)
- **Drenato** se temperature <-5°C prolungate
- **Rete aria** per evitare ghiaccio
- **Fluido glicole** contro gelo (-20°C protezione)

## Schema Circuito Solare

```
Pannello SPCF 100L
┌───────────────────┐
│ Assorbitore rame  │ ← Radiazione solare 800 W/m²
│ Copertura vetro   │
│ Isolamento 50mm   │
└────┬──────────┬───┘
     │          │
Mandata         Ritorno
(80°C)          (40°C)
     │          │
     ▼          ▲
  ┌──────────────────┐
  │   Pompa solare   │ ← Attiva se T_collettore > T_boiler + 5°C
  │   (3 bar max)    │
  └──────────────────┘
     │
     ▼
  ┌────────────────────────┐
  │ Scambiatore di calore  │
  │ (integrato in boiler)  │
  │                        │
  │ IN: 80°C (solare)      │
  │ OUT: 40°C → collettore │
  │                        │
  │ Pre-riscalda ACS       │
  │ a 60°C senza pompa     │
  └────────────────────────┘
```

## Regolazione Automatica

**Termostato solare integrato**:
- Attiva pompa quando T_collettore > T_boiler + 5°C
- Spegne pompa quando Δt < 3°C
- Protezione antigelo automatica
- Massima pressione 3 bar

## Manutenzione

### Annuale
- ✅ Controllare isolamento retro (lana minerale integra)
- ✅ Pulire copertura vetro (pioggia acida, polvere riducono efficienza)
- ✅ Controllare guarnizioni rame (per perdite)
- ✅ Verificare pressione sistema (0.5-1.5 bar in funzione)

### Ogni 5 anni
- 🔄 Controllo fluido termovettore (colore, viscosità)
- 🔄 Eventuale sostituzione glicole (se degradato)
- 🔄 Disincrostazione serpentino scambiatore

### Invernale
- ❄️ Drenaggio se temperature <-5°C prolungate
- ❄️ Protezione contro gelo (fluido glicole sufficiente)

## Rendimento Stagionale

| Stagione | Radiazione | T Collettore | Rendimento | Carico ACS |
|----------|-----------|--------------|-----------|-----------|
| **Giugno-Agosto** | Alto (800 W/m²) | 60-80°C | 65-75% | 80-100% |
| **Maggio/Settembre** | Medio (600 W/m²) | 45-60°C | 55-65% | 40-60% |
| **Aprile/Ottobre** | Basso (400 W/m²) | 35-50°C | 35-45% | 20-30% |
| **Novembre-Marzo** | Molto basso (<200 W/m²) | <35°C | 10-20% | 0-10% |

## Risparmio Energetico Annuale

Stima per impianto Casa Molinella:
- **Consumo ACS senza solare**: ~3000 kWh/anno (pompa calore)
- **Riduzione solare**: ~600-900 kWh/anno (20-30%)
- **Risparmio economico**: €120-180/anno @ €0.20/kWh

## Anomalie

| Sintomo | Causa | Soluzione |
|---------|-------|-----------|
| Pompa non attiva | T_collettore < T_boiler + 5°C | Normale (sole debole) |
| Perdita acqua | Guarnizione rotta | Sostituire O-ring rame |
| Pressione bassa | Perdita circuito | Reintegrare fluido glicole |
| Resa bassa | Vetro sporco/opaco | Pulizia con acqua demineralizzata |
| Ghiaccio collettore | Glicole insufficiente | Verificare concentrazione |

## Documentazione Ufficiale

- [Viessmann Solarcell SPCF 100L](https://www.viessmann.it)
- Manuale impianto: Da richiedere a installatore
- Certificazione EN 12975 (prestazioni garantite)

---

*Fonte: Viessmann Italia - Prodotti Solari Termici*
*Aggiornamento: Aprile 2026*
