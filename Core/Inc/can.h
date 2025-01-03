/*******************************************************************************
 * @file can.h
 * @brief General driver abstracting STM32 HAL: CAN.
 *******************************************************************************
 */

#ifndef NERVE__CAN_H
#define NERVE__CAN_H

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
 * @breif Struct define a message signal.
 */
typedef struct {
  uint8_t start_bit;  // Start bit-position in the CAN message payload.
                      // (0-63 for 8-byte CAN).
  uint8_t bit_length; // Length of the signal in bits.
                      // (1-8 for classic CAN).
  uint8_t byte_order; // Byte order: 0 = Little Endian, 1 = Big Endian.
  float scale;        // Scaling factor to convert raw value to physical value.
  float offset;       // Offset to apply after scaling.
  float min_value;    // Minimum physical value (for validation, optional).
  float max_value;    // Maximum physical value (for validation, optional).
  float value;        // Decoded signal value (after applying scale and offset).
} can_signal_t;

/**
 * @breif Struct define a message and additional handler functions.
 *
 * @note Example member values:
 *  message_id=0x123, id_mask=0xFFF, DLC=n -> Specific ID: 0x100.
 *  message_id=0x200, id_mask=0xFF0, DLC=n -> ID range: 0x200 to 0x20F.
 *  message_id=0x000, id_mask=0x000, DLC=0 -> Default (no DLC check).
 */
typedef struct {
  uint32_t message_id; // CAN message ID.
  uint32_t id_mask;    // Mask for ID filtering (for ranges or specific IDs).
  uint8_t dlc;         // Data Length Code.
  can_message_handler handler; // Pointer to the handler function.
  can_signal_t *signals;       // Array of signals in this message.
  uint8_t signal_count;        // Number of signals in the message.
} can_message_handler_t;

/** Public functions. *********************************************************/

/**
 * @brief Initialize CAN.
 */
void can_init(void);

#endif
