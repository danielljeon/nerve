/**
 *******************************************************************************
 * @file : driver_bno085.c
 * @brief: Driver for 9-DOF IMU (BNO085).
 *******************************************************************************
 * @note:
 * Code developed with Adafruit 9-DOF Orientation IMU Fusion Breakout - BNO085.
 *******************************************************************************
 */

/* Includes. */

#include "driver_bno085.h"

/* Variables. */

volatile BNO085_State BNO085_CURRENT_STATE;

/* Functions. */

/**
 * @brief  Trigger wake procedure from MCU for BNO085.
 */
void BNO085_Trigger_Wake() {
  // Pull BNO085 wake pin low.
  HAL_GPIO_WritePin(BNO085_WAKE_GPIO, BNO085_WAKE_GPIO_PIN, GPIO_PIN_RESET);
  BNO085_CURRENT_STATE = STATE_WAKEUP_STARTED;
}

/**
 * @brief  Process INTN part of wake procedure from BNO085 to MCU.
 */
void BNO085_INTN_EXTI_ISR_Process() {
  BNO085_CURRENT_STATE = STATE_INTN_RECIVED;
}

/**
 * @brief  BNO085 start SPI SHTP communication transmits.
 */
HAL_StatusTypeDef BNO085_SPI_TX() {
  if (BNO085_CURRENT_STATE != STATE_INTN_RECIVED) {
    return HAL_BUSY; // Wait for INTN recive.
  }

  return HAL_OK;
}
