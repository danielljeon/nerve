/*******************************************************************************
 * @file ublox_hal_uart.c
 * @brief u-blox NMEA protocol functions: abstracting STM32 HAL primary UART.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "ublox_hal_uart.h"

/** User implementations of STM32 UART HAL (overwriting HAL). *****************/

// TODO: xbee_api_hal_uart.c/h implements HAL_UART_RxCpltCallback() as well.
//  Need to find a clean way to implement for both the u-blox and XBee modules.
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//  if (huart == &UBLOX_HUART) {
//    if (ublox_buffer[0] == '\n') {
//      nmea_sentence[nmea_index] = '\0'; // Null-terminate the sentence.
//      nmea_index = 0;                   // Reset for next sentence.
//      new_sentence_ready = 1; // Flag that a new NMEA sentence is ready.
//    } else {
//      nmea_sentence[nmea_index++] = ublox_buffer[0]; // Add to buffer.
//      if (nmea_index >= NMEA_BUFFER_SIZE) {
//        nmea_index = 0; // Reset if buffer overflows.
//      }
//    }
//    // Continue receiving.
//    HAL_UART_Receive_IT(huart, ublox_buffer, 1);
//  }
//}

/** Public functions. *********************************************************/

void process_ublox_nmea_data(void) {
  if (nmea_sentence[0] != '\0') { // New NMEA sentence is ready.
    // TODO: Implement string parsing for Standard NMEA messages.

    nmea_sentence[0] = '\0'; // Reset sentence buffer.
    new_sentence_ready = 0;  // Reset flag after processing.
  }
}
