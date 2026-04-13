# Collegamento RS485 — Test Vitocal con FT232RNL da Mac

Guida per testare la comunicazione **Modbus RTU** con la Vitocal 100-S/111-S usando il convertitore **FT232RNL** (USB→RS485) direttamente dal Mac, **senza ESP32**.

---

## Il Problema: Connettori Diversi

```
FT232RNL                    Vitocal (Vitotronic 200)
┌─────────────┐             ┌─────────────────────┐
│ Morsetti    │             │ Porta RS485          │
│ ┌──┬──┬──┐  │             │ Connettore: RJ45     │
│ │A+│B-│GND│ │             │ (come ethernet)      │
│ └──┴──┴──┘  │             └─────────────────────┘
└─────────────┘
       ↑                              ↑
  Hai già questo              Come collegarlo?
```

**Soluzione**: un semplice cavo ethernet tagliato (€1-2).

---

## Opzione 1 — Cavo Ethernet Tagliato ✅ Consigliata

### Cosa Serve
- Cavo ethernet **CAT5/CAT6** qualsiasi, 50cm (probabilmente hai già in casa)
- Forbici/tronchese

### Come Fare
1. Taglia **un'estremità** del cavo
2. Pela i fili per 1cm
3. Collega ai morsetti del FT232RNL

### Mappa Colori (standard EIA-568B)

```
RJ45                    FT232RNL
Pin 1 → Arancio/Bianco → A+
Pin 2 → Arancio        → B−
Pin 3 → Verde/Bianco   → GND
(altri fili: non connettere)
```

```
  RJ45 (vista frontale)
  ┌─────────────────┐
  │ 1 2 3 4 5 6 7 8 │
  └─────────────────┘
    ↑ ↑ ↑
    │ │ └── Verde/Bianco → GND
    │ └──── Arancio      → B−
    └────── Arancio/Bianco → A+
```

- **Costo**: ~€1-2
- **Tempo**: 5 minuti

---

## Opzione 2 — RJ45 Breakout Board (€5-8)

Adattatore professionale: **RJ45 femmina → morsetti a vite** per tutti gli 8 pin.

```
[RJ45 femmina] → [morsetti pin 1…8]
                        ↑
              Collega pin 1→A+, pin 2→B−, pin 3→GND
```

Cerca su Amazon: `RJ45 breakout board` oppure `RJ45 terminal block adapter`

---

## Schema Completo

```
Mac
 │ USB
 ▼
FT232RNL
 │ switch su RS485 (se presente)
 │ morsetti: A+ / B− / GND
 ▼
Cavo ethernet tagliato
 │ fili: arancio/bianco, arancio, verde/bianco
 ▼
RJ45 → porta RS485 Vitotronic 200
 │ (lato destro unità interna Vitocal)
 ▼
Vitocal 100-S / 111-S
```

> ⚠️ **Nota**: questa è la porta **RS485 Modbus RTU** (connettore RJ45 sul Vitotronic 200).
> È **separata** dalla porta Optolink usata da Vitoconnect.

---

## Confronto con il Prodotto RS485 TO ETH B (€32,99)

Potresti aver visto su Amazon il **Waveshare RS485 TO ETH B** (€32,99). È un prodotto diverso:

| | **Cavo ethernet tagliato** | **RS485 TO ETH B** |
|---|---|---|
| **Costo** | ~€1-2 | €32,99 |
| **Alimentazione** | Nessuna | 9-24V esterna |
| **Uso** | Test da Mac | Server permanente in rete |
| **Come funziona** | Mac→USB→FT232RNL→RS485 | RS485→Ethernet LAN |
| **Quando serve** | Test iniziale ✅ | Integrazione HA permanente |

Il **RS485 TO ETH B** è utile **in futuro** se vuoi integrare la Vitocal in Home Assistant via rete LAN senza tenere il Mac collegato.

---

## Risorse Community — Registri Modbus Vitocal

> ⚠️ **IMPORTANTE**: Viessmann **non pubblica** la mappa dei registri Modbus.
> Tutto quello che segue viene da **reverse engineering** della community.
> I registri potrebbero variare tra modelli diversi. Verificare sempre sul campo.

### Protocollo Confermato (da più progetti community)

| Parametro | Valore |
|-----------|--------|
| **Baudrate** | 9600 baud |
| **Parity** | **Even** (E) ← importante! |
| **Stop bits** | 1 |
| **Data bits** | 8 |
| **Slave address** | 1 (fisso) |

### Registri Noti (da Epyon01P/viessmann — reverse engineered)

> ⚠️ Testato su Vitocal non specificato. Potrebbero differire sulla 100-S.

| Registro | Tipo | Descrizione | Formula |
|---------|------|-------------|---------|
| **200** | Lettura | Modalità operativa sistema | valore diretto |
| **400** | Lettura | Temperatura ritorno acqua | ÷ 10 = °C |
| **401** | Lettura | Temperatura mandata acqua | ÷ 10 = °C |
| **405** | Lettura | Temperatura ACS | ÷ 10 = °C |
| **428** | Lettura | Temperatura aria esterna | ÷ 10 = °C |
| **440** | Lettura | Temperatura sistema esterno | ÷ 10 = °C |
| **1002** | Scrittura | Setpoint riscaldamento | ÷ 10 = °C |
| **1003** | Scrittura | Setpoint ACS | ÷ 10 = °C |

### Progetti Community di Riferimento

| Progetto | Linguaggio | Cosa fa | Link |
|---------|-----------|---------|------|
| **Epyon01P/viessmann** | Python | Legge registri + pubblica MQTT | [GitHub](https://github.com/Epyon01P/viessmann) |
| **maga-1/mqtt2vitocal** | Python | Ottimizzazione PV + controllo | [GitHub](https://github.com/maga-1/mqtt2vitocal) |
| **ezplanet/heatpump** | Go | Monitor passivo Modbus → JSON | [GitHub](https://github.com/ezplanet/heatpump) |
| **openv/openv** | Python | Optolink (non Modbus) ma registri utili | [GitHub](https://github.com/openv/openv) |

### Opzione Ufficiale — Vitogate 300

Viessmann vende il **Vitogate 300**: gateway ufficiale che converte CAN-Bus → Modbus TCP/RTU con registro map completo. Costo elevato, ma documentazione ufficiale.

---

## Setup Mac (Driver + Software)

### 1. Driver FTDI (se non installato)
Scarica da: https://ftdichip.com/drivers/vcp-drivers/ → macOS

### 2. Verifica Porta
```bash
ls /dev/tty.* | grep usb
# → /dev/tty.usbserial-XXXXX
```

### 3. Installa pymodbus
```bash
pip install pymodbus
```

---

## Test Modbus RTU — Codice Python

> ⚠️ Nota: parity = **'E'** (Even), non 'N'! Errore comune.

### Test Connessione Base
```python
from pymodbus.client import ModbusSerialClient

client = ModbusSerialClient(
    'serial',
    port='/dev/tty.usbserial-XXXXX',  # cambia con la tua porta
    baudrate=9600,
    parity='E',      # Even parity — obbligatorio!
    stopbits=1,
    bytesize=8,
    timeout=1
)

if client.connect():
    print("✅ Connesso!")
    r = client.read_holding_registers(401, 1, slave=1)
    if not r.isError():
        print(f"✅ T_mandata: {r.registers[0]/10}°C")
    else:
        print("❌ Nessuna risposta (registri sbagliati?)")
else:
    print("❌ Porta non trovata")

client.close()
```

### Lettura Temperature (registri community)
```python
from pymodbus.client import ModbusSerialClient

client = ModbusSerialClient(
    'serial', port='/dev/tty.usbserial-XXXXX',
    baudrate=9600, parity='E', stopbits=1, bytesize=8, timeout=1
)
client.connect()

registri = {
    400: "T_ritorno",
    401: "T_mandata",
    405: "T_ACS",
    428: "T_esterna",
    440: "T_sistema_esterno",
}

for reg, nome in registri.items():
    r = client.read_holding_registers(reg, 1, slave=1)
    if not r.isError():
        print(f"{nome}: {r.registers[0]/10}°C")
    else:
        print(f"{nome}: ❌ errore (REG {reg})")

client.close()
```

### Modalità Operativa
```python
r = client.read_holding_registers(200, 1, slave=1)
if not r.isError():
    print(f"Modalità: {r.registers[0]}")
```

---

## Troubleshooting

| Problema | Causa | Soluzione |
|----------|-------|-----------|
| Porta non trovata | Driver non installato | Installa FTDI VCP driver |
| Nessuna risposta | Parity sbagliata | Usa `parity='E'` (Even) |
| Nessuna risposta | A+/B- invertiti | Scambia i fili A+ e B- |
| Errore CRC | Baud rate sbagliato | Verifica 9600 baud |
| Timeout | Slave address sbagliato | Prova `slave=1` |
| Letture strane | GND non connesso | Aggiungi filo GND |
| Registri = errore | Modello diverso | Prova range 200-500, 1000-1100 |

---

*Aggiornamento: Aprile 2026*
*Fonti: Epyon01P/viessmann, maga-1/mqtt2vitocal, ezplanet/heatpump (GitHub)*
