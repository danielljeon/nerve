/*******************************************************************************
 * @file can.c
 * @brief General driver abstracting STM32 HAL: CAN.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "can.h"
#include "can_nerve.h"
#include "diagnostics.h"

/** Definitions. **************************************************************/

#define MAX_CAN_MESSAGES 10

/** Private variables. ********************************************************/

// Transmit variables.
CAN_TxHeaderTypeDef tx_header;
uint8_t tx_buffer[8];
uint32_t tx_mailbox;

/** Private functions. ********************************************************/

/**
 * @brief Process CAN bus messages based on the configured message structs.
 *
 * This function iterates over the statically allocated CAN bus messages and
 * checks whether the incoming message's ID and DLC match the configuration.
 * If a match is found and a receiver handler exists, that handler is invoked.
 *
 * @param header Pointer to the CAN RX header.
 * @param data Pointer to the raw data of the CAN message.
 */
void process_can_message(CAN_RxHeaderTypeDef *header, uint8_t *data) {
  for (int i = 0; i < dbc_message_count; i++) {
    // Check if the message ID matches.
    if ((header->StdId & dbc_messages[i].id_mask) ==
        dbc_messages[i].message_id) {

      // Check if the message DLC matches, or if no check required (dlc == 0).
      if (dbc_messages[i].dlc == 0 || header->DLC == dbc_messages[i].dlc) {

        // Call the rx_handler if it exists.
        if (dbc_messages[i].rx_handler) {
          dbc_messages[i].rx_handler(header, data);
        }

      } else {
        // Handle ID/DLC mismatch fault.
        can_fault();
        return;
      }
    }
  }
}

/**
 * @brief Extract a signal value from a CAN message payload.
 *
 * This function extracts the raw signal value from the provided data array
 * using the specified bit-position, length, and byte order. It then applies
 * scaling and offset to convert the raw value into a physical value.
 *
 * @param signal Pointer to the CAN signal configuration.
 * @param data Pointer to the raw CAN message payload.
 *
 * @return The decoded physical signal value.
 */
float decode_signal(const can_signal_t *signal, const uint8_t *data) {
  uint64_t raw_value = 0;

  // Extract raw bits from the CAN message payload.
  for (int i = 0; i < signal->bit_length; i++) {
    int bit_position = signal->start_bit + i;
    int byte_index = bit_position / 8;
    int bit_index = bit_position % 8;

    // Use the enumerated constant for byte order.
    if (signal->byte_order == CAN_BIG_ENDIAN) {
      raw_value |= ((data[byte_index] >> (7 - bit_index)) & 0x1)
                   << (signal->bit_length - 1 - i);
    } else { // CAN_LITTLE_ENDIAN.
      raw_value |= ((data[byte_index] >> bit_index) & 0x1) << i;
    }
  }

  // Convert raw value to physical value by applying scale and offset.
  return ((float)raw_value * signal->scale) + signal->offset;
}

/** User implementations of STM32 CAN NVIC HAL (overwriting HAL). *************/

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  // Receive the CAN message.
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK) {
    // Process the received message.
    process_can_message(&rx_header, rx_data);
  }
}

/** Public functions. *********************************************************/

void can_init(void) {
  // Configure CAN bus filters.
  CAN_FilterTypeDef can_filter_config;

  can_filter_config.FilterActivation = CAN_FILTER_ENABLE; // Enable the filter.
  can_filter_config.FilterBank = 18; // Assign filter to bank 18.
  can_filter_config.FilterFIFOAssignment = CAN_FILTER_FIFO0; // Use FIFO0.
  // Set filter to accept all IDs.
  can_filter_config.FilterIdHigh = 0x0000;
  can_filter_config.FilterIdLow = 0x0000;
  // Set mask to 0 so that no bits are filtered, allow all messages.
  can_filter_config.FilterMaskIdHigh = 0x0000;
  can_filter_config.FilterMaskIdLow = 0x0000;
  // Use mask mode (compare message ID with mask).
  can_filter_config.FilterMode = CAN_FILTERMODE_IDMASK;
  // Use 32-bit filter scale.
  can_filter_config.FilterScale = CAN_FILTERSCALE_32BIT;
  // Filter bank config for dual CAN setups.
  can_filter_config.SlaveStartFilterBank = 20;

  // Apply filter settings to both CAN1 and CAN2.
  HAL_CAN_ConfigFilter(&hcan1, &can_filter_config);
  HAL_CAN_ConfigFilter(&hcan2, &can_filter_config);

  // Start CAN1 and CAN2.
  HAL_CAN_Start(&hcan1);
  HAL_CAN_Start(&hcan2);

  // Enable interrupts.
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);

  // Setup CAN general members on the transmit header.
  tx_header.IDE = CAN_ID_STD;
  // Remaining members (DLC and StdId) are configured per message on transmit.
}

static void can_encode_signal(const can_signal_t *signal, uint8_t *data,
                              const float physical_value) {
  // Convert physical value to a raw integer value (use a simple rounding).
  const uint64_t raw_value =
      (uint64_t)(((physical_value - signal->offset) / signal->scale) + 0.5f);

  // Loop over each bit of the signal.
  for (int i = 0; i < signal->bit_length; i++) {
    const int bit_position = signal->start_bit + i;
    const int byte_index = bit_position / 8;
    const int bit_index = bit_position % 8;

    if (signal->byte_order == CAN_BIG_ENDIAN) {
      // For big-endian, fill from the high-order bit.
      data[byte_index] |= ((raw_value >> (signal->bit_length - 1 - i)) & 0x01)
                          << (7 - bit_index);
    } else { // CAN_LITTLE_ENDIAN.
      data[byte_index] |= ((raw_value >> i) & 0x01) << bit_index;
    }
  }
}

HAL_StatusTypeDef can_send_message_generic(const can_message_t *msg,
                                           const float signal_values[]) {
  uint8_t data[8] = {0}; // Initialize the data buffer to 0.

  // Encode each signal value into the data array.
  for (int i = 0; i < msg->signal_count; i++) {
    can_encode_signal(&msg->signals[i], data, signal_values[i]);
  }

  tx_header.StdId = msg->message_id; // Prepare the CAN transmit header.
  tx_header.IDE = CAN_ID_STD;
  tx_header.RTR = CAN_RTR_DATA;
  tx_header.DLC = msg->dlc;

  // Transmit the CAN message on CAN1.
  return HAL_CAN_AddTxMessage(&hcan1, &tx_header, data, &tx_mailbox);
}
