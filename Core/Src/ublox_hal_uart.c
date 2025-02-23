/*******************************************************************************
 * @file ublox_hal_uart.c
 * @brief u-blox NMEA protocol functions: abstracting STM32 HAL: UART.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "ublox_hal_uart.h"
#include <stdlib.h>
#include <string.h>

/** Private variables. ********************************************************/

// Buffer for UART reception.
static uint8_t ublox_rx_buffer[UBLOX_RX_BUFFER_SIZE];
static uint8_t ublox_rx_byte;
static uint16_t ublox_rx_index = 0;

// Latest GPS data.
ublox_data_t gps_data = {0};

static void ublox_process_byte(uint8_t byte);
static void parse_nmea_sentence(char *sentence);

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
double to_decimal_deg(const char *coordinate, const char *direction) {
  // Parse the degrees and minutes.
  double value = atof(coordinate);
  int degrees = (int)(value / 100);
  double minutes = value - (degrees * 100);

  // Convert to decimal degrees.
  double decimal_degrees = degrees + (minutes / 60.0);

  // Apply direction correction (negative for S or W).
  if (direction[0] == 'S' || direction[0] == 'W') {
    decimal_degrees = -decimal_degrees;
  }

  return decimal_degrees;
}

/**
 * @brief Process incoming u-blox UART data byte.
 *
 * @param byte Byte value to process.
 */
static void ublox_process_byte(uint8_t byte) {
  // Append byte to buffer.
  if (ublox_rx_index < UBLOX_RX_BUFFER_SIZE - 1) {
    ublox_rx_buffer[ublox_rx_index++] = byte;

    // Check for end of NMEA sentence.
    if (byte == '\n') {
      ublox_rx_buffer[ublox_rx_index] = '\0'; // Null-terminate the string
      parse_nmea_sentence((char *)ublox_rx_buffer);
      ublox_rx_index = 0; // Reset buffer index.
    }
  } else {
    ublox_rx_index = 0; // Buffer overflow, reset index.
  }
}

/**
 * @brief Process incoming u-blox UART NMEA sentences.
 *
 * @param sentence NMEA sentence to parse.
 */
static void parse_nmea_sentence(char *sentence) {
  // Check for GNGGA sentence (Global Positioning System Fix Data).
  if (strncmp(sentence, "$GNGGA", 6) == 0) {
    // Buffer for tokenizing.
    char buffer[100];
    strncpy(buffer, sentence, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    // Tokenize the NMEA sentence.
    const char *delimiter = ",";
    char *token = strtok(buffer, delimiter);
    int field_index = 0;

    // Store temporary values before decimal degree conversion.
    char raw_latitude[15] = {0};
    char raw_longitude[15] = {0};

    while (token != NULL) {
      switch (field_index) {
      case 1: // Time (UTC).
        strncpy(gps_data.time, token, sizeof(gps_data.time) - 1);
        break;
      case 2: // Latitude.
        strncpy(raw_latitude, token, sizeof(raw_latitude) - 1);
        break;
      case 3: // Latitude Direction.
        strncpy(gps_data.lat_dir, token, sizeof(gps_data.lat_dir) - 1);
        break;
      case 4: // Longitude.
        strncpy(raw_longitude, token, sizeof(raw_longitude) - 1);
        break;
      case 5: // Longitude Direction.
        strncpy(gps_data.lon_dir, token, sizeof(gps_data.lon_dir) - 1);
        break;
      case 6: // Fix Quality.
        gps_data.fix_quality = atoi(token);
        break;
      case 7: // Number of Satellites.
        gps_data.satellites = atoi(token);
        break;
      case 8: // Horizontal Dilution of Precision.
        gps_data.hdop = atof(token);
        break;
      case 9: // Altitude.
        gps_data.altitude = atof(token);
        break;
      case 11: // Geoidal Separation.
        gps_data.geoid_sep = atof(token);
        break;
      default:
        ublox_error_handler();
        break;
      }
      field_index++;
      token = strtok(NULL, delimiter);
    }
    // Convert latitude and longitude to decimal degrees.
    gps_data.latitude = to_decimal_deg(raw_latitude, gps_data.lat_dir);
    gps_data.longitude = to_decimal_deg(raw_longitude, gps_data.lon_dir);
  }
}

/** User implementations of STM32 UART HAL (overwriting HAL). *****************/

void HAL_UART_RxCpltCallback_ublox(UART_HandleTypeDef *huart) {
  if (huart == &UBLOX_HUART) {
    // Process the received byte.
    ublox_process_byte(ublox_rx_byte);

    // Receive the next byte.
    HAL_UART_Receive_IT(&UBLOX_HUART, &ublox_rx_byte, 1);
  }
}

/** Public functions. *********************************************************/

void ublox_init(void) {
  // Ensure the u-blox module is not in reset state.
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_SET);

  // Start UART reception in interrupt mode.
  HAL_UART_Receive_IT(&UBLOX_HUART, &ublox_rx_byte, 1);
}

void ublox_reset(void) {
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_RESET);
  HAL_Delay(5); // Hold reset for 5 ms.
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_SET);
}
