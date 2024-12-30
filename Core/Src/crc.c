/** Includes. *****************************************************************/

#include "crc.h"

/** Public functions. *********************************************************/

static inline uint8_t crc8_dvb_s2_update(uint8_t crc, uint8_t data) {
  crc ^= data;
  for (int i = 0; i < 8; i++) {
    if (crc & 0x80) {
      crc = (crc << 1) ^ 0xD5; // Polynomial = 1101 0101 (0xD5).
    } else {
      crc <<= 1;
    }
  }
  return crc;
}

uint8_t crc8_dvb_s2(const uint8_t *data, size_t length) {
  uint8_t crc = 0;
  for (size_t i = 0; i < length; i++) {
    crc = crc8_dvb_s2_update(crc, data[i]);
  }
  return crc;
}
