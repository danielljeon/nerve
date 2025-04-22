/*******************************************************************************
 * @file ublox_hal_uart.c
 * @brief u-blox NMEA protocol functions: abstracting STM32 HAL: UART.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "ublox_hal_uart.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/** Definitions. **************************************************************/

#define UBLOX_RX_BUFFER_SIZE 256

#define GNGGA_TOKEN_COUNT 12

/** Private variables. ********************************************************/

// Buffer for UART reception.
static uint8_t ublox_rx_buffer[UBLOX_RX_BUFFER_SIZE];

// Latest GPS data.
ublox_data_t gps_data = {0};

// Rx buffer management for DMA based operation.
static uint16_t ublox_rx_index = 0;
static bool ublox_in_sentence = false;
static bool ublox_first_half = true;

/** Private functions. ********************************************************/

/**
 * @brief Generic void error handler.
 */
void ublox_error_handler(void) { gps_fault(); }

/**
 * @brief Convert latitude/longitude from DDMM.MMMM to decimal degrees.
 *
 * @param coordinate Original degrees and minutes measurements.
 * @param direction Original direction measurement (N, S, E, W).
 *
 * @return Converted decimal degrees measurement.
 */
double to_decimal_deg(const char *coordinate, const char direction) {
  // Parse the degrees and minutes.
  double value = strtod(coordinate, NULL);
  int degrees = (int)(value / 100);
  double minutes = value - (degrees * 100);

  // Convert to decimal degrees.
  double decimal_degrees = degrees + (minutes / 60.0);

  // Apply direction correction (negative for S or W).
  if (direction == 'S' || direction == 'W') {
    decimal_degrees = -decimal_degrees;
  }

  return decimal_degrees;
}

/**
 * @brief XOR of all chars between `$` and `*`, return parsed checksum.
 *
 * @param sentence NMEA sentence to process.
 *
 * @return bool
 * @retval == true -> Checksum pass.
 * @retval == false -> Checksum fail.
 */
static bool validate_nmea_checksum(const char *sentence) {
  const char *p = sentence;
  if (*p != '$')
    return false;
  uint8_t checksum = 0;
  while (*++p && *p != '*' && *p != '\r' && *p != '\n') {
    checksum ^= (uint8_t)*p;
  }
  if (*p != '*')
    return false;

  // Parse the two hex digits after '*'.
  char hexbuf[3] = {p[1], p[2], 0};
  char *endptr = NULL;
  uint8_t received = (uint8_t)strtoul(hexbuf, &endptr, 16);
  if (endptr != &hexbuf[2])
    return false;

  return checksum == received;
}

/** @brief Parse GNGGA fields.
 *
 * @return bool
 * @retval == true -> All values seem valid.
 * @retval == false -> At least 1 value seems invalid.
 *
 * @note GPS data is still updated on failure using information processed up to
 * (but not including) the invalid information.
 */
static bool parse_gngga(const char *sentence) {
  // Quick tokenization in-place.
  char buf[UBLOX_RX_BUFFER_SIZE];
  size_t len = strnlen(sentence, sizeof(buf) - 1);
  if (len >= sizeof(buf) - 1)
    return false;
  memcpy(buf, sentence, len);
  buf[len] = '\0';

  // Split into comma‑delimited tokens.
  char *tokens[GNGGA_TOKEN_COUNT] = {0};
  char *saveptr = NULL;
  tokens[0] = strtok_r(buf, ",", &saveptr);
  for (int i = 1; i < GNGGA_TOKEN_COUNT && tokens[i - 1]; ++i) {
    tokens[i] = strtok_r(NULL, ",", &saveptr);
  }
  // Must have at least fields up through geoid separation (index 11).
  if (!tokens[0] || !tokens[11])
    return false;

  // Field conversions.
  // UTC time.
  strncpy(gps_data.time, tokens[1], sizeof(gps_data.time) - 1);
  gps_data.time[sizeof(gps_data.time) - 1] = '\0';

  // Latitude and longitude.
  gps_data.latitude = to_decimal_deg(tokens[2], tokens[3][0]);
  gps_data.lon_dir = tokens[5][0];
  gps_data.longitude = to_decimal_deg(tokens[4], tokens[5][0]);
  gps_data.lat_dir = tokens[3][0];

  char *endptr = NULL; // Create null pointer for stdlib functions.

  // Fix quality.
  gps_data.fix_quality = (unsigned)strtoul(tokens[6], &endptr, 10);
  if (endptr == tokens[6])
    return false;

  // Number of satellites.
  gps_data.satellites = (unsigned)strtoul(tokens[7], &endptr, 10);
  if (endptr == tokens[7])
    return false;

  // hdop, altitude, geoid separation.
  gps_data.hdop = strtof(tokens[8], &endptr);
  gps_data.altitude = strtod(tokens[9], &endptr);
  gps_data.geoid_sep = strtod(tokens[11], &endptr);

  return true;
}

/**
 * @brief Top‑level sentence dispatcher.
 *
 * @param sentence NMEA sentence to process.
 */
static void parse_nmea_sentence(const char *sentence) {
  if (strncmp(sentence, "$GNGGA", 6) == 0) { // Handle GNGGA sentence.
    if (!validate_nmea_checksum(sentence) || !parse_gngga(sentence)) {
      ublox_error_handler();
    }
  }
  // Ignore other sentence types.
}

/**
 * @brief Process incoming u-blox UART data byte.
 *
 * @param byte Byte value to process.
 */

static void ublox_process_byte(uint8_t byte) {
  if (!ublox_in_sentence) {
    if (byte == '$') { // Start of new sentence.
      ublox_in_sentence = true;
      ublox_rx_index = 0;
      ublox_rx_buffer[ublox_rx_index++] = byte;
    }
    return; // Ignore until start of sentence found.
  }

  if (ublox_rx_index < UBLOX_RX_BUFFER_SIZE - 1) { // Processing a sentence.
    ublox_rx_buffer[ublox_rx_index++] = byte;
  } else { // Overflow, drop sentence and wait for next start of sentence.
    ublox_in_sentence = false;
    ublox_rx_index = 0;
    return;
  }

  if (byte == '\n') {                             // End of sentence received.
    ublox_rx_buffer[ublox_rx_index] = '\0';       // Null‑terminate.
    parse_nmea_sentence((char *)ublox_rx_buffer); // Parse sentence.

    // Reset variables for next sentence.
    ublox_in_sentence = false;
    ublox_rx_index = 0;
  }
}

/** User implementations of STM32 UART HAL (overwriting HAL). *****************/

void HAL_UART_RxCpltCallback_ublox(UART_HandleTypeDef *huart) {
  if (huart == &UBLOX_HUART) {
    // Process the second half when the buffer wraps.
    size_t start = UBLOX_RX_BUFFER_SIZE / 2;
    size_t end = UBLOX_RX_BUFFER_SIZE;

    for (size_t i = start; i < end; ++i) {
      ublox_process_byte(ublox_rx_buffer[i]);
    }
    ublox_first_half = !ublox_first_half;
  }
}

void HAL_UART_RxHalfCpltCallback_ublox(UART_HandleTypeDef *huart) {
  if (huart == &UBLOX_HUART) {
    // Process the first half.
    for (size_t i = 0; i < UBLOX_RX_BUFFER_SIZE / 2; ++i) {
      ublox_process_byte(ublox_rx_buffer[i]);
    }
    ublox_first_half = !ublox_first_half;
  }
}

/** Public functions. *********************************************************/

void ublox_init(void) {
  // Ensure the u-blox module is not in reset state.
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_SET);

  // Start UART reception with DMA.
  HAL_UART_Receive_DMA(&UBLOX_HUART, ublox_rx_buffer, UBLOX_RX_BUFFER_SIZE);
}

void ublox_reset(void) {
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_RESET);
  HAL_Delay(5); // Hold reset for 5 ms.
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_SET);
}
