/*******************************************************************************
 * @file xbee_api_hal_uart.c
 * @brief XBee API: abstracting STM32 HAL primiary UART.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "xbee_api_hal_uart.h"

/** Definitions. **************************************************************/

#define XBEE_TX_BUFFER_SIZE 128 // Opinionated conservative value for overhead.
#define DMA_RX_BUFFER_SIZE 256

#define START_DELIMITER 0x7E
#define FRAME_TYPE_TX_REQUEST 0x10

#define BROADCAST_RADIUS 0x00     // 0x00 = Maximum hops.
#define FRAME_ID_WITH_STATUS 0x01 // Non-zero to track status.
#define FRAME_ID_NO_STATUS 0x00   // No status tracking for non-critical data.
#define OPTIONS_WITH_ACK 0x00     // Request ACK.
#define OPTIONS_NO_ACK 0x01       // Disable ACK for faster transmission.

#define TRANSMIT_STATUS 0x8B // Transmit Status (0x8B) confirming delivery.

/** Private variables. ********************************************************/

xbee_api_buffer_t api_buf; // Declare XBee API buffer struct.

uint8_t rx_dma_buffer[DMA_RX_BUFFER_SIZE]; // Circular buffer for DMA.

volatile uint16_t rx_read_index = 0;  // Points to where the CPU has processed.
volatile uint16_t rx_write_index = 0; // Points to where DMA has written.

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
 * @param frame: Full XBee API frame with 0x8B transmit status header.
 */
void handle_transmit_status(const uint8_t *frame) {
  const uint8_t delivery_status = frame[5]; // Extract delivery status byte.

  if (delivery_status == 0x00) { // Success: frame delivered successfully.
    // TODO: Process success.
  } else { // Failure: frame delivery failed.
    // TODO: Process failure, retry/log the error.
  }
}

/**
 * @brief Process complete Rx XBee API frames.
 *
 * @param frame: Full XBee API frame.
 * @param length: :Length of the XBee API frame.
 */
void process_complete_frame(const uint8_t *frame, uint16_t length) {
  // Decode the frame, check frame type, payload, etc.
  const uint8_t frame_type = frame[0];
  if (frame_type == TRANSMIT_STATUS) {
    handle_transmit_status(frame);
  } else {
    // TODO: Process failure, retry/log the error.
  }
}

/**
 * @brief Process incoming data bytes of XBee API frames via DMA UART.
 *
 * @param byte: Data byte of XBee API frame.
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
    frame_length = (byte << 8); // Store high byte of length.
    frame_state = WAIT_LENGTH_LOW;
    break;

  case WAIT_LENGTH_LOW:
    frame_length |= byte; // Store low byte of length.
    if (frame_length > XBEE_TX_BUFFER_SIZE) {
      // Invalid frame length, reset state.
      frame_state = WAIT_START_DELIMITER;
    } else {
      frame_state = WAIT_FRAME_DATA;
    }
    break;

  case WAIT_FRAME_DATA:
    frame_buffer[frame_index++] = byte;
    if (frame_index == frame_length) {
      // Frame complete, process it.
      process_complete_frame(frame_buffer, frame_length);
      frame_state = WAIT_START_DELIMITER; // Reset for next frame.
    }
    break;
  }
}

/**
 * @breif Prase data for 0x8B Transmit Status frames and other messages.
 *
 * @param data: Recived data.
 * @param length: Length of data.
 */
void process_dma_data(uint8_t *data, const uint16_t length) {
  while (rx_read_index != rx_write_index) {
    // Check for a complete frame in the buffer.
    const uint8_t data_byte = rx_dma_buffer[rx_read_index];

    // Process the byte (e.g., part of an XBee API frame).
    handle_incoming_byte(data_byte);

    // Increment the read pointer.
    rx_read_index++;
    if (rx_read_index >= DMA_RX_BUFFER_SIZE) {
      rx_read_index = 0;
    }
  }
}

/** User implemntations of STM32 DMA HAL (overwritting HAL). ******************/

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &XBEE_HUART) {
    // Process the first half of the buffer.
    process_dma_data(rx_dma_buffer, DMA_RX_BUFFER_SIZE / 2);
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
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
  xbee_api_buffer_t api_buf;

  // Initialize the API buffer.
  init_xbee_api_buffer(&api_buf, buffer, sizeof(buffer));

  // Add frame type.
  add_byte(&api_buf, FRAME_TYPE_TX_REQUEST);

  // Set frame ID for status track conditions based on criticality.
  if (is_critical) {
    add_byte(&api_buf, FRAME_ID_WITH_STATUS);
  } else {
    add_byte(&api_buf, FRAME_ID_NO_STATUS);
  }

  // Add 64-bit destination address.
  add_bytes(&api_buf,
            (uint8_t[]){(dest_addr >> 56) & 0xFF, (dest_addr >> 48) & 0xFF,
                        (dest_addr >> 40) & 0xFF, (dest_addr >> 32) & 0xFF,
                        (dest_addr >> 24) & 0xFF, (dest_addr >> 16) & 0xFF,
                        (dest_addr >> 8) & 0xFF, dest_addr & 0xFF},
            8);

  // Add 16-bit network address.
  add_byte(&api_buf, (dest_net_addr >> 8) & 0xFF); // High byte.
  add_byte(&api_buf, dest_net_addr & 0xFF);        // Low byte.

  // Add boardcast hop radius.
  add_byte(&api_buf, BROADCAST_RADIUS);

  // Set options for ACK based on criticality.
  if (is_critical) {
    add_byte(&api_buf, OPTIONS_WITH_ACK); // Request ACK for critical.
  } else {
    add_byte(&api_buf, OPTIONS_NO_ACK); // Disable ACK for non-critical.
  }

  // Add the payload.
  add_bytes(&api_buf, payload, payload_size);
  finalize_api_frame(&api_buf); // Finalize (add length and checksum).

  // Send the frame via UART.
  HAL_UART_Transmit_DMA(&XBEE_HUART, api_buf.buffer, api_buf.index);
}
