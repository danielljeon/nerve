/*******************************************************************************
 * @file ublox_hal_uart.h
 * @brief u-blox NMEA protocol functions: abstracting STM32 HAL: UART.
 *******************************************************************************
 */

#ifndef NERVE__UBLOX_HAL_UART_H
#define NERVE__UBLOX_HAL_UART_H

/** Includes. *****************************************************************/

#include "diagnostics.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include <stdio.h>
#include <string.h>

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

// Define a struct to store GPS data.
typedef struct {
  char time[10];    // UTC Time.
  double latitude;  // Latitude in decimal degrees.
  char lat_dir[2];  // Latitude Direction (N/S).
  double longitude; // Longitude in decimal degrees.
  char lon_dir[2];  // Longitude Direction (E/W).
  int fix_quality;  // GPS Fix Quality.
  //  0 = No fix.
  //  1 = Autonomous GNSS fix.
  //  2 = Differential GNSS fix.
  //  4 = RTK fixed.
  //  5 = RTK float.
  //  6 = Estimated/dead reckoning fix.
  int satellites;  // Number of Satellites.
  float hdop;      // Horizontal Dilution of Precision.
  float altitude;  // Altitude in meters.
  float geoid_sep; // Geoidal Separation.
} ublox_data_t;

/** Public variables. *********************************************************/

extern ublox_data_t gps_data;

/** User implementations of STM32 NVIC HAL (overwriting HAL). *****************/

void HAL_UART_RxCpltCallback_ublox(UART_HandleTypeDef *huart);

/** Public functions. *********************************************************/

/**
 * @breif Initialize the u-blox module.
 */
void ublox_init(void);

/**
 * @breif Reset the u-blox module.
 *
 * @note: Only for critical situations for recovery, triggers cold start.
 */
void ublox_reset(void);

#endif
