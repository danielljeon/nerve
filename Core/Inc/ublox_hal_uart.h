/*******************************************************************************
 * @file ublox_hal_uart.h
 * @brief u-blox NMEA protocol functions: abstracting STM32 HAL: UART.
 *******************************************************************************
 */

#ifndef NERVE__UBLOX_HAL_UART_H
#define NERVE__UBLOX_HAL_UART_H

/** Includes. *****************************************************************/

#include <stdio.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"

/** Definitions. **************************************************************/

#define UBLOX_RX_BUFFER_SIZE 512

/** STM32 port and pin configs. ***********************************************/

extern UART_HandleTypeDef huart2;

// UART.
#define UBLOX_HUART huart2

// GPIO output for reset.
#define UBLOX_RESETN_PORT GPIOA
#define UBLOX_RESETN_PIN GPIO_PIN_1

/** Public structs ************************************************************/

typedef struct {
  float latitude;
  float longitude;
  float altitude;
  uint8_t fix_type; // GPS fix type.
} ublox_coordinates_t;

/** Public variables. *********************************************************/

extern ublox_coordinates_t latest_coordinates;

/** User implementations of STM32 NVIC HAL (overwriting HAL). *****************/

void HAL_UART_RxCpltCallback_ublox(UART_HandleTypeDef *huart);

/** Public functions. *********************************************************/

/**
 * @breif Initialize the u-blox module.
 */
void ublox_init(void);

/**
 * @breif Reset the u-blox module.
 */
void ublox_reset(void);

/**
 * @breif Process received NMEA sentences.
 */
ublox_coordinates_t ublox_get_coordinates(void);

#endif
