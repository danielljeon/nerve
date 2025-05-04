/*******************************************************************************
 * @file callbacks.c
 * @brief STM32 HAL callback implementations overriding weak declarations.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "can.h"
#include "sh2_hal_spi.h"
#include "stm32f4xx_hal.h"
#include "ublox_hal_uart.h"

/** Collection of user implementations of STM32 HAL (overwriting HAL). ********/

/** GPIO. */

void HAL_GPIO_EXTI_Callback(uint16_t n) { HAL_GPIO_EXTI_Callback_sh2(n); }

/** UART. */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  HAL_UART_RxCpltCallback_ublox(huart);
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {}

/** SPI. */

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  HAL_SPI_TxRxCpltCallback_sh2(hspi);
}

/** CAN. */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  HAL_CAN_RxFifo0MsgPendingCallback_can(hcan);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  HAL_CAN_RxFifo1MsgPendingCallback_can(hcan);
}
