/*******************************************************************************
 * @file can.h
 * @brief General driver abstracting STM32 HAL: CAN.
 *******************************************************************************
 */

#ifndef __SH2_HAL_SPI_H
#define __SH2_HAL_SPI_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_can.h"

/** STM32 port and pin configs. ***********************************************/

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/** Public structs. ***********************************************************/
/**
 * @breif Define a function pointer type for processing CAN messages.
 *
 * @param header: CAN RX header (CAN_RxHeaderTypeDef).
 * @param data: uint8_t CAN RX data array.
 */
typedef void (*can_message_handler)(CAN_RxHeaderTypeDef *header, uint8_t *data);

/**
 * @breif Struct to map a message type (ID or range) to a handler function.
 */
typedef struct {
  uint32_t message_id;  // CAN message ID.
  uint32_t id_mask;     // Mask for ID filtering (for ranges or specific IDs).
  uint8_t expected_dlc; // Expected Data Length Code.
  can_message_handler handler; // Pointer to the handler function.
} can_message_type;

/** Public functions. *********************************************************/

/**
 * @brief Initialize CAN.
 */
void can_init(void);

#endif
