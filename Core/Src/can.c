/*******************************************************************************
 * @file can.c
 * @brief General driver abstracting STM32 HAL: CAN.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "can.h"
#include "diagnostics.h"

/** Definitions. **************************************************************/

#define MAX_CAN_HANDLERS 10

/** Private variables. ********************************************************/

// Transmit variables.
CAN_TxHeaderTypeDef tx_header;
uint8_t tx_buffer[8];
uint32_t tx_mailbox;

can_message_handler_t can_message_handlers[MAX_CAN_HANDLERS] = {};

/** Private functions. ********************************************************/

/**
 * @breif Process CAN messages based on handler struct members.
 *
 * @param header CAN RX header type def.
 * @param data Data to process.
 */
void process_can_message(CAN_RxHeaderTypeDef *header, uint8_t *data) {
  for (int i = 0; i < MAX_CAN_HANDLERS; i++) {

    // Check if the message ID matches.
    if ((header->StdId & can_message_handlers[i].id_mask) ==
        can_message_handlers[i].message_id) {

      // Check if the message DLC matches.
      if (can_message_handlers[i].dlc == 0 ||
          header->DLC == can_message_handlers[i].dlc) {

        // Call the handler if it exists.
        if (can_message_handlers[i].handler) {
          can_message_handlers[i].handler(header, data);
        }
        return;

      } else {
        // Handle ID/DLC mismatch fault.
        can_fault();
        return;
      }
    }
  }
}

/**
 * @breif Extract a signal value from a CAN message payload.
 *
 * @param signal Signal message to decode based on.
 * @param data Raw data to decode.
 *
 * @return Decoded physical signal value.
 */
float decode_signal(const can_signal_t *signal, const uint8_t *data) {
  uint64_t raw_value = 0;

  // Extract raw bits from CAN message payload.
  for (int i = 0; i < signal->bit_length; i++) {
    int bit_position = signal->start_bit + i;
    int byte_index = bit_position / 8;
    int bit_index = bit_position % 8;

    if (signal->byte_order == 1) { // Big Endian.
      raw_value |= ((data[byte_index] >> (7 - bit_index)) & 0x1)
                   << (signal->bit_length - 1 - i);
    } else { // Little Endian.
      raw_value |= ((data[byte_index] >> bit_index) & 0x1) << i;
    }
  }

  // Convert raw value to physical value.
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
