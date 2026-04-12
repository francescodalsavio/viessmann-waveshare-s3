# Quadro Interruttori - Distribuzione Energia Sistema Riscaldamento

**Fonte:** Viessmann - Documentazione tecnica sistema Vitocal

## Panoramica

**Quadro interruttori e distribuzione elettrica** per il sistema Vitocal. Pannello di controllo principale che distribuisce l'alimentazione alle componenti del sistema:
- Pompa di calore (Vitocal 100-S)
- Scaldacqua pompa di calore (Vitocal 060-A)
- Boiler di accumulo termico
- Ventilconvettori (Fan-coil)
- Pompa solare (circuito pannello termico)
- Controller Modbus/centrale di comando

## Funzionamento

### Modalità Operativa

| Modalità | Descrizione |
|----------|------------|
| **Pompa Giorno** | Riscaldamento/Raffrescamento normale durante ore diurne |
| **Pompa Notte** | Modulazione ridotta durante ore notturne (ridotto consumo) |
| **Stagione Invernale** | Priorità riscaldamento (pompa calore + integrazione solare) |
| **Stagione Estiva** | Priorità raffrescamento + ACS da solare |
| **Solare** | Attivazione circuito pannello termico (Δt > 5°C) |
| **Pompa Calore** | Funzionamento pompa aria-acqua per riscaldamento/raffrescamento |

## Protezione Circuiti

```
Alimentazione Principale (230V/400V monofase+trifase)
       │
       ├─→ [Interruttore generale] ← Protezione principale
       │
       ├─→ Circuito Pompa di Calore (Vitocal 100-S)
       │        └─ Protezione: Interruttore automatico + relè termica
       │
       ├─→ Circuito Scaldacqua (Vitocal 060-A)
       │        └─ Protezione: Interruttore automatico
       │
       ├─→ Circuito Boiler/Pompa Circolazione
       │        └─ Protezione: Interruttore automatico
       │
       ├─→ Circuito Ventilconvettori
       │        └─ Protezione: Interruttore automatico
       │
       ├─→ Circuito Pompa Solare
       │        └─ Protezione: Interruttore automatico
       │
       └─→ Circuito Controllo Modbus
                └─ Protezione: Interruttore automatico
```

## Caratteristiche di Sicurezza

✅ **Protezione sovraccorrente** - Interruttori magnetotermici su ogni circuito
✅ **Protezione contatti differenziali** - Salvavita (RCD) per sicurezza utente
✅ **Conformità VDE** - Standard tedesco di installazione elettrica
✅ **Design modulare** - Componenti facilmente sostituibili
✅ **Fusibili e relè** - Protezione ridondante per affidabilità
✅ **Collegamento pre-assemblato** - Riduce errori di cablaggio

## Operazione Sistema

### Accensione Sistema
1. Interruttore generale → **ON**
2. Interruttori singoli circuiti → **ON** (pompa calore, boiler, circolazione)
3. Led di controllo → Verifica alimentazione
4. Avvio pompa di calore → Automatico

### Spegnimento Emergenza
1. Interruttore generale → **OFF** (disconnette tutto)
2. Tutti i circuiti si disattivano
3. Timer di protezione: 5 min prima di riavvio (protezione compressore)

### Commutazione Stagione Inverno/Estate
- **Inverno**: Pompa giorno + pompa notte modulata (80-100% carico)
- **Estate**: Pompa raffrescamento + solare prioritario (ridotto carico pompa)
- Commutazione automatica o manuale via controller Modbus

## Manutenzione

### Mensile
- ✅ Verifica LED di alimentazione (verde = ok)
- ✅ Controllo assenza anomalie visive

### Annuale
- 🔄 Verifica integrità cablaggio (nessuna ossidazione)
- 🔄 Test funzionamento interruttori
- 🔄 Controllo serraggi terminali

### Se Guasto
- ❌ Non tentare riparazioni se non specializzato
- ❌ Contattare tecnico Viessmann certificato
- ❌ Scaricare energia prima di interventi (OFF per 5 min)

## Standard Conformità

- **VDE 0100** - Installazioni elettriche bassa tensione (Germania)
- **DIN EN 60204-32** - Sicurezza macchine - Quadri di controllo
- **CE** - Marcatura conformità europea
- **RoHS** - Restrizioni sostanze pericolose

## Documentazione Ufficiale

- [Viessmann - Custom Control Panels](https://www.viessmann.ca/en/products/system-technology/custom-controls.html)
- [Viessmann Installation Instructions](https://www.viessmann-us.com/en/services/downloads/manuals.html)
- Standard VDE per installazioni elettriche

---

*Fonte: Viessmann - Documentazione Tecnica Sistemi Vitocal*
*Aggiornamento: Aprile 2026*
