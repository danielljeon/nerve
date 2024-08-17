/**
*******************************************************************************
* @file : driver_bno085.h
* @brief: Driver for 9-DOF IMU (BNO085).
*******************************************************************************
* @note:
* Code developed with Adafruit 9-DOF Orientation IMU Fusion Breakout - BNO085.
*******************************************************************************
*/

#ifndef __DRIVER_BNO0885_H
#define __DRIVER_BNO0885_H

#include "stm32h7xx_hal.h"

/* Definitions. */

#define BNO085_SPI hspi4
#define BNO085_WAKE_GPIO GPIOF
#define BNO085_WAKE_GPIO_PIN GPIO_PIN_1
#define BNO085_INTN_GPIO GPIOF
#define BNO085_INTN_GPIO_PIN GPIO_PIN_2
#define GNO085_NRST_GPIO GPIOF
#define GNO085_NRST_GPIO_PIN GPIO_PIN_3

/**
 * @brief State machine to model BNO085 communication.
 */
typedef enum {
  STATE_IDLE,
  STATE_WAKEUP_STARTED,
  STATE_WAIT_FOR_INTN,
  STATE_INTN_RECIVED,
  STATE_SPI_SHTP_COMM,
  STATE_SPI_COMPLETE
} BNO085_State;

/* Variables. */

extern volatile BNO085_State BNO085_CURRENT_STATE;
extern volatile uint8_t BNO085_INTN_RECEIVED;

/* Functions. */

/**
 * @brief  Trigger wake procedure from MCU for BNO085.
 */
void BNO085_Trigger_Wake();

/**
 * @brief  Process INTN part of wake procedure from BNO085 to MCU.
 */
void BNO085_INTN_EXTI_ISR_Process();

/**
 * @brief  BNO085 start SPI SHTP communication transmits.
 */
HAL_StatusTypeDef BNO085_SPI_TX();

#endif
