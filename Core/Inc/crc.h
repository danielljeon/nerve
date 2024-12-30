/*******************************************************************************
 * @file crc.h
 * @brief CRC: Cyclic Redundancy Check calculation functions.
 *******************************************************************************
 */

#ifndef NERVE__CRC_H
#define NERVE__CRC_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** Public functions. *********************************************************/

/**
 * @brief Update CRC-8 (DVB-S2) with one byte.
 *
 * @param crc  Previous CRC value.
 * @param data Byte to process.
 * @return Updated CRC value.
 */
static inline uint8_t crc8_dvb_s2_update(uint8_t crc, uint8_t data);

/**
 * @brief Calculate CRC-8 (DVB-S2) for an entire buffer (bitwise method).
 *
 * @param data Pointer to the input data.
 * @param length Number of bytes in the data buffer.
 * @return CRC-8 result.
 */
uint8_t crc8_dvb_s2(const uint8_t *data, size_t length);

#endif
