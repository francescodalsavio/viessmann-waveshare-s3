# Dati Sniffati - Master Originale Viessmann

Questo folder contiene i dati catturati dal thermostat master originale Viessmann tramite sniffer Modbus RS485.

## File

- `master_sequenza_acceso_spento.json` — Sequenza di accensione/spegnimento ripetuta
- `master_test_accensione.json` — Test di accensione con keep-alive periodico

## Struttura JSON

```json
{
  "num": "1",           // Numero sequenziale comando
  "tempo": "01:25.911", // Timestamp assoluto (mm:ss.ms)
  "registro": "0.00s",  // Offset dal comando precedente
  "valore": "101",      // Numero registro (101, 102, 103)
  "decodifica": "0x4003", // Valore hex inviato
  "azione": ""          // Descrizione (facoltativa)
}
```

## Pattern Osservato

### Accensione/Spegnimento
```
REG 101 (0x4003/0x4083) → +1.16s → REG 102 (0xe6) → +1.15s → REG 103 (0xaf)
```

### Keep-Alive
- Intervallo: **~68 secondi**
- Invia: sempre i 3 registri nello stesso ordine
- Funzione: mantiene sincronizzazione dispositivo

## Registri

| REG | Funzione | Valori |
|-----|----------|--------|
| 101 | Potenza + Mode + Fan | 0x4003 (ON), 0x4083 (OFF) |
| 102 | Temperatura | 0xe6 (23.0°C), varia con impostazione |
| 103 | Modalità operativa | 0xaf (valore master) |

## Note Implementazione

- ✅ Delay 1.16s tra registri implementato
- ✅ Keep-alive 68s implementato  
- ✅ REG 103 = 0xaf (matchato con master)
- ✅ Sequenza registri corretta: 101 → 102 → 103
