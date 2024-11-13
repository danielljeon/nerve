/*******************************************************************************
 * @file ublox_hal_uart.h
 * @brief u-blox NMEA protocol functions: abstracting STM32 HAL primary UART.
 *******************************************************************************
 */

#ifndef __UBLOX_HAL_UART_H
#define __UBLOX_HAL_UART_H

/** Includes. *****************************************************************/

#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

/** Definitions. **************************************************************/

#define UBLOX_UART_RX_BUFFER_SIZE 512
#define NMEA_BUFFER_SIZE 128

/** STM32 port and pin configs. ***********************************************/

extern UART_HandleTypeDef huart2;

// SPI.
#define UBLOX_HUART huart2

// GPIO output for reset.
#define UBLOX_RESETN_PORT GPIOA
#define UBLOX_RESETN_PIN GPIO_PIN_1

/** Public variables. *********************************************************/

extern uint8_t ublox_buffer[UBLOX_UART_RX_BUFFER_SIZE]; // Raw UART data buffer.
extern uint8_t nmea_sentence[NMEA_BUFFER_SIZE];         // NMEA sentence buffer.
extern volatile uint16_t nmea_index;
extern volatile uint8_t new_sentence_ready;

/** User implementations of STM32 UART HAL (overwriting HAL). *****************/

/**
 * @brief STM32 HAL abstraction initialization.
 *
 * @param huart: Structure instance of bmp3_dev.
 *
 * @return Status of execution.
 * @retval 0 -> Success.
 * @retval < 0 -> Failure info.
 */
// TODO: xbee_api_hal_uart.c/h implements HAL_UART_RxCpltCallback() as well.
//  Need to find a clean way to implement for both the u-blox and XBee modules.
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/** Public functions. *********************************************************/

/**
 * @brief Process and parse NMEA data from ublox buffers.
 */
void process_ublox_nmea_data(void);

#endif
