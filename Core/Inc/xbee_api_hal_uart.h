/*******************************************************************************
 * @file xbee_api_hal_uart.h
 * @brief XBee API: abstracting STM32 HAL: UART.
 *******************************************************************************
 */

#ifndef NERVE__XBEE_HAL_UART_H
#define NERVE__XBEE_HAL_UART_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** STM32 port and pin configs. ***********************************************/

extern UART_HandleTypeDef huart1;

// U(S)ART.
#define XBEE_HUART huart1

// GPIO output for reset.
#define XBEE_NRST_PORT GPIOA
#define XBEE_NRST_PIN GPIO_PIN_15

/** Definitions. **************************************************************/

#define XBEE_TX_BUFFER_SIZE 128 // Opinionated value for Tx buffer.
#define XBEE_RX_BUFFER_SIZE 32  // Opinionated value for Rx buffer.

/** Public types. *************************************************************/

/**
 * @brief Structure to manage the XBee API buffer.
 */
typedef struct {
  uint8_t *buffer; // Pointer to the buffer.
  uint16_t size;   // Total buffer size.
  uint16_t index;  // Current index in the buffer.
} xbee_api_buffer_t;

/** User implementations of STM32 DMA HAL (overwriting HAL). ******************/

void USART1_IRQHandler_xbee(UART_HandleTypeDef *huart);

/** Public functions. *********************************************************/

/**
 * @brief Initialize the XBee radio module.
 */
void xbee_init(void);

/**
 * @brief Reset the XBee radio module.
 */
void xbee_reset(void);

/**
 * @brief Send a message over XBee API.
 *
 * This function prepares messages in the XBee API frame format.
 * Transmission using UART with DMA for non-blocking transmission.
 *
 * @param dest_addr 64-bit address of the destination XBee device/node.
 * @param dest_net_addr 16-bit network address of the destination device.
 * @param payload Pointer to the data buffer containing the payload to be sent.
 * @param payload_size The size of the payload in bytes.
 * @param is_critical Determines if the message is critical or non-critical. If
 * set to a non-zero value, the message is marked as critical and will request
 * an acknowledgment (ACK) from the recipient. If set to zero, the message is
 * non-critical and no acknowledgment is required.
 */
void xbee_send(uint64_t dest_addr, uint16_t dest_net_addr,
               const uint8_t *payload, uint16_t payload_size,
               uint8_t is_critical);

#endif
