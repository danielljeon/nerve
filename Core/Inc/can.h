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

/** Definitions. **************************************************************/

#define MAX_SIGNALS_PER_MESSAGE 8 // Maximum signals allowed per message.

/** STM32 port and pin configs. ***********************************************/

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/** Public structs. ***********************************************************/

/**
 * @brief Define function pointer types for processing CAN bus messages.
 *
 * Two distinct handlers are defined:
 *   1. can_rx_handler_t: For decoding received CAN messages.
 *   2. can_tx_handler_t: For encoding data into CAN messages for transmission.
 */
typedef void (*can_rx_handler_t)(CAN_RxHeaderTypeDef *header, uint8_t *data);
typedef void (*can_tx_handler_t)(uint8_t *data_out);

/**
 * @brief Enumeration for CAN signal byte order.
 */
typedef enum {
  CAN_LITTLE_ENDIAN = 0, // Little Endian byte order.
  CAN_BIG_ENDIAN = 1     // Big Endian byte order.
} can_byte_order_t;

/**
 * @brief Struct defining a CAN message signal.
 *
 * This struct describes an individual signal within a CAN message.
 * It includes fields for bit-position, length, scaling, and validation.
 * An optional name field aids in debugging and logging.
 */
typedef struct {
  const char *name;            // Optional signal identifier (for debugging).
  uint8_t start_bit;           // Start bit-position (0-63 for 8-byte CAN).
  uint8_t bit_length;          // Length of the signal in bits.
  can_byte_order_t byte_order; // Byte order: little or big endian.
  float scale;         // Scaling factor to convert raw value to physical value.
  float offset;        // Offset to apply after scaling.
  float min_value;     // Minimum physical value (optional validation).
  float max_value;     // Maximum physical value (optional validation).
  float decoded_value; // Decoded signal value (scale and offset).
} can_signal_t;

/**
 * @brief Struct defining a CAN message configuration.
 *
 * This struct holds the static configuration for a CAN message, including its
 * ID, optional name, data length, and the associated handler functions. Signals
 * for the message are statically allocated in a fixed-size array.
 */
typedef struct {
  const char *name;    // Optional message name (for debugging).
  uint32_t message_id; // CAN message ID.
  uint32_t id_mask; // ID mask for filtering (supports ranges or specific IDs).
  uint8_t dlc;      // Data Length Code.
  can_rx_handler_t rx_handler; // Function pointer for receiving (decoding).
  can_tx_handler_t tx_handler; // Function pointer for transmitting (encoding).
  can_signal_t signals[MAX_SIGNALS_PER_MESSAGE]; // Statically allocation.
  uint8_t signal_count; // Number of valid signals in the array.
} can_message_t;

/** Public functions. *********************************************************/

/**
 * @brief Initialize CAN.
 */
void can_init(void);

#endif
