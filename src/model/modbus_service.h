#ifndef MODBUS_SERVICE_H
#define MODBUS_SERVICE_H

#ifdef ARDUINO
  #include <Arduino.h>
#endif
#include <cstdint>

/**
 * ModbusService - Layer per comunicazione Modbus ASCII RS485
 *
 * Responsabilità:
 * - Calcolazione LRC checksum
 * - Invio frame Modbus ASCII
 * - Parsing frame Modbus (per sniffer mode)
 *
 * NON sa niente di logica di business o UI.
 */

#ifdef ARDUINO
  #define RS485 Serial1
#endif
#define BAUD_RATE 9600
#define RS485_TX_PIN 44
#define RS485_RX_PIN 43

struct ModbusFrame {
  uint8_t addr;
  uint8_t func;
  uint16_t reg;
  uint16_t val;
  uint8_t lrc;
  bool lrc_ok;
  uint32_t timestamp;
};

class ModbusService {
private:
  uint8_t calculateLRC(uint8_t *data, int len) {
    uint8_t lrc = 0;
    for (int i = 0; i < len; i++) lrc += data[i];
    return (uint8_t)(-(int8_t)lrc);
  }

public:
  ModbusService() {}

  void begin() {
#ifdef ARDUINO
    RS485.begin(BAUD_RATE, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    delay(100);
#endif
  }

  /**
   * Invia un singolo registro Modbus
   * @param addr Indirizzo slave (0x00 = broadcast)
   * @param reg Numero registro (0x0065 = REG 101, 0x0066 = REG 102, etc)
   * @param value Valore a 16-bit
   */
  void writeRegister(uint8_t addr, uint16_t reg, uint16_t value) {
    uint8_t payload[6];
    payload[0] = addr;
    payload[1] = 0x06;  // Function code: Write Single Register
    payload[2] = (reg >> 8) & 0xFF;
    payload[3] = reg & 0xFF;
    payload[4] = (value >> 8) & 0xFF;
    payload[5] = value & 0xFF;

    uint8_t lrc = calculateLRC(payload, 6);

    // Costruisci frame ASCII: :AAFFRRRRRRRRLLCC\r\n
    char txBuf[32];
    int pos = 0;
    txBuf[pos++] = ':';
    for (int i = 0; i < 6; i++) {
      pos += sprintf(&txBuf[pos], "%02X", payload[i]);
    }
    pos += sprintf(&txBuf[pos], "%02X", lrc);
    txBuf[pos++] = '\r';
    txBuf[pos++] = '\n';

#ifdef ARDUINO
    // Invia
    while (RS485.available()) RS485.read();  // Svuota buffer
    RS485.write((uint8_t*)txBuf, pos);
    RS485.flush();

    Serial.printf("[MODBUS] Inviato: addr=0x%02X reg=0x%04X val=0x%04X lrc=0x%02X\n",
                  addr, reg, value, lrc);
#endif
  }

  /**
   * Parsa un frame Modbus ASCII ricevuto
   * @param hexStr String esadecimale ricevuto (senza ':' e '\r\n')
   * @return true se parsing riuscito
   */
  bool parseFrame(const char* hexStr, ModbusFrame* frame) {
    int len = strlen(hexStr);
    if (len < 14) return false;

    uint8_t data[7];
    for (int i = 0; i < 7 && i * 2 < len; i++) {
      char hex[3] = {hexStr[i * 2], hexStr[i * 2 + 1], 0};
      data[i] = (uint8_t)strtol(hex, NULL, 16);
    }

    frame->addr = data[0];
    frame->func = data[1];
    frame->reg = (data[2] << 8) | data[3];
    frame->val = (data[4] << 8) | data[5];
    frame->lrc = data[6];
#ifdef ARDUINO
    frame->timestamp = millis();
#else
    frame->timestamp = 0;
#endif

    // Verifica LRC
    uint8_t lrc_calc = calculateLRC(data, 6);
    frame->lrc_ok = (frame->lrc == lrc_calc);

    return true;
  }

  uint16_t regAddress(int regNumber) {
    return 0x0064 + regNumber;  // REG 101 = 0x0065, REG 102 = 0x0066, etc
  }
};

#endif
