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

can_message_type can_message_handlers[MAX_CAN_HANDLERS] = {
    // Examples:
    // {0x123, 0xFFF, 8, handle_type_a},   // Specific ID: 0x100, DLC=8.
    // {0x200, 0xFF0, 4, handle_type_b},   // ID range: 0x200 to 0x20F, DLC=4.
    // {0x000, 0x000, 0, handler_default}, // Default (DLC = 0, no DLC check).
};

/** Private functions. ********************************************************/

void process_can_message(CAN_RxHeaderTypeDef *header, uint8_t *data) {
  for (int i = 0; i < MAX_CAN_HANDLERS; i++) {

    // Check if the message ID matches.
    if ((header->StdId & can_message_handlers[i].id_mask) ==
        can_message_handlers[i].message_id) {

      // Check if the message DLC matches.
      if (can_message_handlers[i].expected_dlc == 0 ||
          header->DLC == can_message_handlers[i].expected_dlc) {

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
  // Start CAN.
  HAL_CAN_Start(&hcan1);
  HAL_CAN_Start(&hcan2);

  // Enable interrupts.
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
  HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);

  // Setup CAN general members on the transmit header.
  tx_header.IDE = CAN_ID_STD;
  // Remaining members (DLC and StdId) are configured per message on transmit.
}
