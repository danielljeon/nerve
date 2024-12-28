/*******************************************************************************
 * @file callbacks.c
 * @brief STM32 HAL callback implementations overriding weak declarations.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"
#include "ublox_hal_uart.h"
#include "xbee_api_hal_uart.h"

/** Collection of user implementations of STM32 UART HAL (overwriting HAL). ***/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  HAL_UART_RxCpltCallback_xbee(huart);
  HAL_UART_RxCpltCallback_ublox(huart);
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
  HAL_UART_RxHalfCpltCallback_xbee(huart);
}
