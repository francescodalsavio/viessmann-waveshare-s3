# Pannelli Solari Termici - Sistema Riscaldamento Casa Molinella

**Fonte:** Documentazione ufficiale Viessmann

## Panoramica Sistema Solare

Casa Molinella dispone di **2 pannelli solari termici** per pre-riscaldamento acqua calda sanitaria (ACS) e riduzione carico pompa di calore:

1. **SPCF 100L** - Collettore piano (installazione principale)
2. **Vitosol 300-TM** - Pannello sottovuoto ad alta efficienza (integrazione avanzata)

## Confronto Pannelli Solari

| Aspetto | SPCF 100L (Piano) | Vitosol 300-TM (Sottovuoto) |
|---------|---|---|
| **Tipo** | Collettore piano selettivo | Heat pipe sottovuoto |
| **Tecnologia** | Vetro + rame assorbente | Tubo sottovuoto + Duotec |
| **Rendimento** | 65-75% | 80-85% |
| **Assorbitore** | Rame selettivo fisso | Assorbitore orientabile +/-25° |
| **Isolamento** | Lana minerale 50mm | Sottovuoto (eccellente) |
| **Protezione gelo** | Glicole manuale (controllato) | Automatica heat pipe (-20°C) |
| **Surriscaldo** | Manuale (controllato) | ThermProtect brevettato |
| **Montaggio** | Tetto inclinato (fisso) | Verticale/orizzontale (flessibile) |
| **Manutenzione** | Annuale, pulizia vetro | Annuale, tubi sostituibili a pressione |
| **Efficienza bassa stagione** | Media | Superiore |
| **Costo** | Minore | Maggiore |

## Sistema Solare Integrato

```
CIRCUITO SOLARE AUTONOMO (indipendente da riscaldamento/raffrescamento)

Pannelli Solari (SPCF 100L + Vitosol 300-TM)
       ↓
Pompa solare (attiva se T_pannello > T_boiler + 5°C)
       ↓
Scambiatore di calore integrato in boiler
       ↓
Pre-riscalda ACS da 15°C a 60-70°C
       ↓
Boiler (SPCF 100L accumulo termico)
       ↓
Pompa di calore (Vitocal 100-S)
├─ Se ACS già calda (dal sole) → integra solo ΔT minore
├─ Se ACS fredda (inverno) → riscalda completamente
└─ Riduce consumo energetico 30-40% in estate
```

## Stagionale: Produttività Solare

| Stagione | SPCF 100L | Vitosol 300-TM | Risultato Sistema |
|----------|---|---|---|
| **Giugno-Agosto** | 80-100% ACS | 100% ACS | ✅ ACS gratuita dal sole |
| **Maggio/Settembre** | 40-60% ACS | 80-90% ACS | ✅ ACS prevalentemente solare |
| **Aprile/Ottobre** | 20-30% ACS | 50-70% ACS | ⚠️ Integrazione pompa calore |
| **Novembre-Marzo** | 0-10% ACS | 10-30% ACS | ❌ Pompa di calore principale |

## Vantaggi Configurazione Dual-Panel

✅ **Produttività estesa** - Vitosol 300 compensa bassa stagione
✅ **Risparmio combinato** - SPCF 100L + Vitosol 300 = 30-50% ACS gratuita annua
✅ **Ridondanza** - Se un pannello guasto, l'altro continua
✅ **Flessibilità** - SPCF fisso + Vitosol orientabile per spazi variegati
✅ **Protezione automtica** - Entrambi hanno protezione surriscaldo
✅ **Integrazione totale** - Circuito autonomo con pompa solare unica

## Manutenzione Annuale (Entrambi)

### SPCF 100L
- ✅ Pulire vetro (pioggia acida, polvere)
- ✅ Controllare isolamento retro
- ✅ Verificare guarnizioni rame
- ✅ Controllare pressione sistema (0,5-1,5 bar)

### Vitosol 300-TM
- ✅ Ispezione visiva vetro tubi
- ✅ Controllo pressione circuito (0,5-1,5 bar)
- ✅ Verifica isolamento tubi sottovuoto
- ✅ Nessuna sedimentazione tubi (fluido heat pipe auto-pulente)

### Protezione Invernale (Automatica)
- ❄️ SPCF 100L: Glicole protezione -5°C
- ❄️ Vitosol 300: Heat pipe protezione -20°C
- ❄️ Pompa solare: Disattivazione automatica se T < 5°C

## Risparmio Energetico Annuale

**Consumo ACS senza solare**: ~3000 kWh/anno

**Con SPCF 100L solo**: 
- Riduzione: ~600-900 kWh/anno (20-30%)
- Risparmio: €120-180/anno @ €0.20/kWh

**Con SPCF 100L + Vitosol 300-TM (configurazione Casa Molinella)**:
- Riduzione: ~900-1500 kWh/anno (30-50%)
- Risparmio: €180-300/anno @ €0.20/kWh
- **Ritorno investimento**: 8-12 anni (Vitosol)

## Anomalie e Soluzioni

| Sintomo | Causa Probabile | Soluzione |
|---------|---|---|
| Pompa solare non attiva | T_pannello < T_boiler + 5°C (sole debole) | Normale - sole insufficiente |
| Perdita acqua | Guarnizione rotta (SPCF) o collegamento (Vitosol) | Contattare tecnico |
| Pressione bassa | Perdita circuito solare | Reintegrare fluido glicole |
| Resa bassa (SPCF) | Vetro sporco/offuscato | Pulizia con acqua demineralizzata |
| Resa bassa (Vitosol) | Tubi sottovuoto offuscati (vapor d'acqua) | Controllare isolamento, possibile ricondensamento |
| Surriscaldo boiler | ThermProtect (Vitosol) / Regolazione (SPCF) | Normale protezione - verificare setpoint |

## Schema Idraulico Completo

```
┌─────────────────────────────────────────┐
│  PANNELLI SOLARI                        │
│  ├─ SPCF 100L (collettore piano)        │
│  └─ Vitosol 300-TM (sottovuoto)         │
└────────┬────────────────────────────────┘
         │ Tubo mandata (80°C)
         ▼
    Pompa solare (attiva se Δt > 5°C)
         │
         ▼
    ┌──────────────────────┐
    │ BOILER ACCUMULO      │
    │ SPCF 100L (97L)      │
    │                      │
    │ Scambiatore solare   │
    │ ├─ IN: 80°C (sole)   │
    │ └─ OUT: 40°C         │
    │                      │
    │ Pre-riscalda ACS     │
    │ a 60°C senza pompa   │
    └───┬──────────────────┘
        │
        ├─ ACS (45-50°C comfort)
        │  ↓
        │  Rubinetti / Servizi
        │
        └─ Riscaldamento (55°C)
           ↓
           Ventilconvettori (2×)
```

## Documentazione Ufficiale

### SPCF 100L (Collettore Piano)
- [Viessmann Solarcell SPCF 100L](https://www.viessmann.it)
- Manuale impianto: Da richiedere a installatore
- Certificazione EN 12975 (prestazioni garantite)

### Vitosol 300-TM (Sottovuoto)
- [Vitosol 300-TM | Viessmann IT](https://www.viessmann.it/it/prodotti/pannello-solare/vitosol-300-tm.html)
- [Brochure Vitosol 300-TM (PDF)](https://www.viessmann.it/content/dam/vi-brands/IT/BROCHURE/2017/Vitosol_300_TM_brochure_pannelli_solari_Viessmann_09_2017.pdf)
- Tecnologia ThermProtect brevettata

---

*Fonte: Documentazione ufficiale Viessmann SPCF 100L e Vitosol 300-TM*
*Configurazione: Casa Molinella - Sistema duale solare*
*Aggiornamento: Aprile 2026*
