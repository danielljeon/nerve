/*******************************************************************************
 * @file xbee_api_hal_uart.c
 * @brief XBee API: abstracting STM32 HAL: UART.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "xbee_api_hal_uart.h"
#include "stm32f4xx_hal.h"

/** Definitions. **************************************************************/

#define XBEE_TX_BUFFER_SIZE 128 // Opinionated conservative value for overhead.

#define START_DELIMITER 0x7E
#define FRAME_TYPE_TX_REQUEST 0x10

#define FRAME_TYPE_TX_REQUEST 0x10 // Transmit Request frame type.
#define FRAME_ID_WITH_STATUS 0x01  // Non-zero Frame ID for ACK.
#define FRAME_ID_NO_STATUS 0x00    // Zero Frame ID, no ACK.
#define BROADCAST_RADIUS 0x00      // Maximum hops.
#define OPTIONS_WITH_ACK 0x00      // Request ACK.
#define OPTIONS_NO_ACK 0x01        // Disable ACK.

#define TRANSMIT_STATUS 0x8B // Transmit Status (0x8B) confirming delivery.

/** Public variables. *********************************************************/

uint8_t rx_dma_buffer[DMA_RX_BUFFER_SIZE]; // Circular buffer for DMA.

/** Private variables. ********************************************************/

volatile uint16_t rx_read_index = 0; // Points to where the CPU has processed.

// XBee API frame reading state machine for DMA UART (Rx) usage.
typedef enum {
  WAIT_START_DELIMITER,
  WAIT_LENGTH_HIGH,
  WAIT_LENGTH_LOW,
  WAIT_FRAME_DATA
} frame_state_t;

// Create state machine instance.
frame_state_t frame_state = WAIT_START_DELIMITER;

// DMA UART (Rx) frame processing variables.
uint8_t frame_buffer[XBEE_TX_BUFFER_SIZE];
uint16_t frame_length = 0;
uint16_t frame_index = 0;

/** Private functions. ********************************************************/

/**
 * @brief Function to add the start delimiter.
 */
void add_start_delimiter(xbee_api_buffer_t *api_buf) {
  if (api_buf->index < api_buf->size) {
    api_buf->buffer[api_buf->index++] = START_DELIMITER;
  }
}

/**
 * @brief Function to initialize the API buffer.
 */
void init_xbee_api_buffer(xbee_api_buffer_t *api_buf, uint8_t *buf,
                          const uint16_t size) {
  api_buf->buffer = buf;
  api_buf->size = size;
  api_buf->index = 0;

  // Add start delimiter and increment buffer index.
  add_start_delimiter(api_buf);

  // Increment the index by 2 to reserve space for length (will be set by
  // update length).
  api_buf->index += 2;
}

/**
 * @brief Function to update the length
 *
 * @note Must be called after the frame is complete.
 */
void update_length(const xbee_api_buffer_t *api_buf) {
  const uint16_t length =
      api_buf->index - 3; // Length is total bytes after length field.
                          // Excludes start delimiter and length.
  api_buf->buffer[1] = (length >> 8) & 0xFF; // High byte.
  api_buf->buffer[2] = length & 0xFF;        // Low byte.
}

/**
 * @brief Function to add a single byte of frame data.
 */
void add_byte(xbee_api_buffer_t *api_buf, const uint8_t byte) {
  if (api_buf->index < api_buf->size) {
    api_buf->buffer[api_buf->index++] = byte;
  }
}

/**
 * @brief Function to add multiple bytes (for payloads).
 */
void add_bytes(xbee_api_buffer_t *api_buf, const uint8_t *data,
               const uint16_t length) {
  for (uint16_t i = 0; i < length && api_buf->index < api_buf->size; ++i) {
    api_buf->buffer[api_buf->index++] = data[i];
  }
}

/**
 * @brief Function to calculate and add the checksum.
 */
void add_checksum(xbee_api_buffer_t *api_buf) {
  uint8_t checksum = 0;
  // Checksum is calculated from frame data (from index 3 to end of the frame).
  for (uint16_t i = 3; i < api_buf->index; ++i) {
    checksum += api_buf->buffer[i];
  }
  checksum = 0xFF - checksum;  // XBee checksum formula.
  add_byte(api_buf, checksum); // Add the checksum at the end of the frame.
}

/**
 * @brief Function to finalize the frame (update length and checksum).
 */
void finalize_api_frame(xbee_api_buffer_t *api_buf) {
  update_length(api_buf); // Update the length in the header.
  add_checksum(api_buf);  // Add the checksum to the end.
}

/**
 *@brief Processing frames with 0x8B transmit status header.
 *
 * @param frame Full XBee API frame with 0x8B transmit status header.
 */
void handle_transmit_status(const uint8_t *frame) {
  // Extract frame type byte.
  const uint8_t delivery_status = frame[5];

  if (delivery_status == 0x00) {
    // Success.
    // TODO: Process success.
  } else {
    // Failure.
    // TODO: Process failure, retry/log the error.
  }
}

/**
 * @brief Process complete Rx XBee API frames.
 *
 * @param frame Full XBee API frame.
 * @param length Length of the XBee API frame.
 */
void process_complete_frame(const uint8_t *frame, uint16_t length) {
  // Ensure length is at least 1 to have a checksum byte.
  if (length < 1) {
    return; // Invalid frame length.
  }

  // Calculate checksum over frame data (excluding checksum).
  uint8_t checksum = 0;
  for (uint16_t i = 0; i < length - 1; ++i) {
    checksum += frame[i];
  }
  checksum = 0xFF - checksum;

  // Compare calculated checksum to received checksum.
  if (checksum != frame[length - 1]) {
    // Checksum error, discard frame.
    return;
  }

  const uint8_t frame_type = frame[0];
  if (frame_type == TRANSMIT_STATUS) {
    handle_transmit_status(frame);
  } else {
    // TODO: Handle other frame types if necessary.
  }
}

/**
 * @brief Process incoming data bytes of XBee API frames via DMA UART.
 *
 * @param byte Data byte of XBee API frame.
 */
void handle_incoming_byte(uint8_t byte) {
  switch (frame_state) {
  case WAIT_START_DELIMITER:
    if (byte == START_DELIMITER) {
      frame_index = 0;
      frame_length = 0;
      frame_state = WAIT_LENGTH_HIGH;
    }
    break;

  case WAIT_LENGTH_HIGH:
    frame_length = byte << 8;
    frame_state = WAIT_LENGTH_LOW;
    break;

  case WAIT_LENGTH_LOW:
    frame_length |= byte;
    if (frame_length > XBEE_TX_BUFFER_SIZE) {
      // Invalid frame length, reset state.
      frame_state = WAIT_START_DELIMITER;
    } else {
      frame_state = WAIT_FRAME_DATA;
    }
    break;

  case WAIT_FRAME_DATA:
    if (frame_index < frame_length + 1) { // + 1 for checksum.
      frame_buffer[frame_index++] = byte;
      if (frame_index == frame_length + 1) {
        // Frame complete, process it.
        process_complete_frame(frame_buffer, frame_length + 1);
        frame_state = WAIT_START_DELIMITER;
      }
    } else {
      // Buffer overflow, reset state.
      frame_state = WAIT_START_DELIMITER;
    }
    break;
  }
}

/**
 * @breif Parse data for 0x8B Transmit Status frames and other messages.
 *
 * @param data Received data.
 * @param length Length of data.
 */
void process_dma_data(const uint8_t *data, uint16_t length) {
  for (uint16_t i = 0; i < length; ++i) {
    uint8_t data_byte = data[(rx_read_index + i) % DMA_RX_BUFFER_SIZE];

    // Process the byte (e.g., part of an XBee API frame).
    handle_incoming_byte(data_byte);
  }

  // Update the read index.
  rx_read_index = (rx_read_index + length) % DMA_RX_BUFFER_SIZE;
}

/** User implementations of STM32 DMA HAL (overwriting HAL). ******************/

void HAL_UART_RxHalfCpltCallback_xbee(UART_HandleTypeDef *huart) {
  if (huart == &XBEE_HUART) {
    // Process the first half of the buffer.
    process_dma_data(rx_dma_buffer, DMA_RX_BUFFER_SIZE / 2);
  }
}

void HAL_UART_RxCpltCallback_xbee(UART_HandleTypeDef *huart) {
  if (huart == &XBEE_HUART) {
    // Process the second half of the buffer.
    process_dma_data(&rx_dma_buffer[DMA_RX_BUFFER_SIZE / 2],
                     DMA_RX_BUFFER_SIZE / 2);
  }
}

/** Public functions. *********************************************************/

void send(const uint64_t dest_addr, const uint16_t dest_net_addr,
          const uint8_t *payload, const uint16_t payload_size,
          const uint8_t is_critical) {
  uint8_t buffer[128];
  xbee_api_buffer_t api_buffer; // Declare the API buffer structure

  // Reset buffer to all zeros.
  for (int i = 0; i < 128; i++) {
    buffer[i] = 0;
  }

  // Initialize the API buffer.
  init_xbee_api_buffer(&api_buffer, buffer, sizeof(buffer));

  // Add frame type (0x10 for Transmit Request).
  add_byte(&api_buffer, FRAME_TYPE_TX_REQUEST);

  // Set Frame ID: Non-zero for ACK-required messages.
  uint8_t frame_id = is_critical ? FRAME_ID_WITH_STATUS : FRAME_ID_NO_STATUS;
  add_byte(&api_buffer, frame_id);

  // Add 64-bit destination address (big-endian).
  uint8_t dest_addr_bytes[8] = {
      (dest_addr >> 56) & 0xFF, (dest_addr >> 48) & 0xFF,
      (dest_addr >> 40) & 0xFF, (dest_addr >> 32) & 0xFF,
      (dest_addr >> 24) & 0xFF, (dest_addr >> 16) & 0xFF,
      (dest_addr >> 8) & 0xFF,  dest_addr & 0xFF};
  add_bytes(&api_buffer, dest_addr_bytes, 8);

  // Add 16-bit network address (big-endian).
  add_byte(&api_buffer, (dest_net_addr >> 8) & 0xFF); // High byte.
  add_byte(&api_buffer, dest_net_addr & 0xFF);        // Low byte.

  // Add broadcast radius (0x00 for maximum hops).
  add_byte(&api_buffer, BROADCAST_RADIUS);

  // Set options: 0x00 to request ACK, 0x01 to disable ACK.
  uint8_t options = is_critical ? OPTIONS_WITH_ACK : OPTIONS_NO_ACK;
  add_byte(&api_buffer, options);

  // Ensure payload fits in the buffer.
  if (payload_size > (sizeof(buffer) - api_buffer.index)) {
    // TODO: Add error handling for payload too large.
    return;
  }

  // Add the payload.
  add_bytes(&api_buffer, payload, payload_size);

  // Finalize the API frame (calculate length and checksum).
  finalize_api_frame(&api_buffer);

  // Send the frame via UART using DMA.
  HAL_UART_Transmit_DMA(&XBEE_HUART, api_buffer.buffer, api_buffer.index);
}
