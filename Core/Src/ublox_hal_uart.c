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

// Latest GPS coordinates.
ublox_coordinates_t latest_coordinates = {0};

static void ublox_process_byte(uint8_t byte);
static void parse_nmea_sentence(char *sentence);

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
  // Configure RESETN pin as output.
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = UBLOX_RESETN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(UBLOX_RESETN_PORT, &GPIO_InitStruct);

  // Reset the u-blox module.
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_RESET);
  HAL_Delay(10); // Hold reset for 10 ms.
  HAL_GPIO_WritePin(UBLOX_RESETN_PORT, UBLOX_RESETN_PIN, GPIO_PIN_SET);

  // Start UART reception in interrupt mode.
  HAL_UART_Receive_IT(&UBLOX_HUART, &ublox_rx_byte, 1);
}

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

static void parse_nmea_sentence(char *sentence) {
  // Check for GNGGA sentence (Global Positioning System Fix Data).
  if (strncmp(sentence, "$GNGGA", 6) == 0) {
    char *token;
    char *rest = sentence;
    int field_num = 0;

    double latitude = 0.0;
    char ns_indicator = 0;
    double longitude = 0.0;
    char ew_indicator = 0;
    int fix_quality = 0;
    double altitude = 0.0;

    while ((token = strtok_r(rest, ",", &rest))) {
      field_num++;
      switch (field_num) {
      case 3: // Latitude.
        latitude = atof(token);
        break;
      case 4: // N/S Indicator.
        ns_indicator = token[0];
        break;
      case 5: // Longitude.
        longitude = atof(token);
        break;
      case 6: // E/W Indicator.
        ew_indicator = token[0];
        break;
      case 7: // Fix Quality.
        fix_quality = atoi(token);
        break;
      case 10: // Altitude.
        altitude = atof(token);
        break;
      default:
        break;
      }
    }

    // Convert latitude and longitude to decimal degrees.
    double lat_deg = (int)(latitude / 100);
    double lat_min = latitude - (lat_deg * 100);
    double lat_dec_deg = lat_deg + (lat_min / 60.0);
    if (ns_indicator == 'S') {
      lat_dec_deg = -lat_dec_deg;
    }

    double lon_deg = (int)(longitude / 100);
    double lon_min = longitude - (lon_deg * 100);
    double lon_dec_deg = lon_deg + (lon_min / 60.0);
    if (ew_indicator == 'W') {
      lon_dec_deg = -lon_dec_deg;
    }

    // Update latest coordinates.
    latest_coordinates.latitude = (float)lat_dec_deg;
    latest_coordinates.longitude = (float)lon_dec_deg;
    latest_coordinates.altitude = (float)altitude;
    latest_coordinates.fix_type = (uint8_t)fix_quality;
  }
}
