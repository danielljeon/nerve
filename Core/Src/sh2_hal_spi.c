/*******************************************************************************
 * @file sh2_hal_spi.c
 * @brief BNO085 SH2 functions: abstracting STM32 HAL: SPI.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "sh2_hal_spi.h"
#include "sh2_err.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/** Private types. ************************************************************/

typedef enum spi_state_e {
  SPI_INIT,
  SPI_DUMMY,
  SPI_IDLE,
  SPI_RD_HDR,
  SPI_RD_BODY,
  SPI_WRITE
} spi_state_t;

/** Private variables. ********************************************************/

// Dummy tx data for SPI reads.
static const uint8_t tx_zeros[SH2_HAL_MAX_TRANSFER_IN] = {0};

// SPI bus access state machine.
static spi_state_t spi_state = SPI_INIT;

// Timestamp of last receive.
static volatile uint32_t rx_timestamp_us;

// True from time SH is put in reset until first INTN indication.
static volatile bool in_reset;

// Set true when INTN is observed, until RX operation starts.
static volatile bool intn_seen_rx_ready;

// Receive support.
static uint8_t rx_buffer[SH2_HAL_MAX_TRANSFER_IN];
static volatile uint32_t rx_buf_len;

// Transmit support.
static uint8_t tx_buffer[SH2_HAL_MAX_TRANSFER_OUT];
static uint32_t tx_buf_len;

// Instances of the SPI HAL for SH2 and DFU.
static sh2_Hal_t sh2Hal;

// SH2 instance open or closed.
static bool is_open = false;

/** Private Functions. ********************************************************/

static void enable_interrupts(void) {
  HAL_NVIC_EnableIRQ(SH2_HSPI_IRQ);
  HAL_NVIC_EnableIRQ(SH2_INTN_EXTI_IRQ);
}

static void disable_interrupts() {
  HAL_NVIC_DisableIRQ(SH2_HSPI_IRQ);
  HAL_NVIC_DisableIRQ(SH2_INTN_EXTI_IRQ);
}

static void cs_write_pin(const GPIO_PinState pin_state) {
  HAL_GPIO_WritePin(SH2_CSN_PORT, SH2_CSN_PIN, pin_state);
}

static void ps0_wake_write_pin(const GPIO_PinState pin_state) {
  HAL_GPIO_WritePin(SH2_PS0_WAKEN_PORT, SH2_PS0_WAKEN_PIN, pin_state);
}

static void rstn_write_pin(const GPIO_PinState pin_state) {
  HAL_GPIO_WritePin(SH2_RSTN_PORT, SH2_RSTN_PIN, pin_state);
}

/**
 * @brief Get the current time in us.
 *
 * TODO: NOTE: Currently, no timer overflow handling!
 */
static uint32_t time_now_us(void) { return __HAL_TIM_GET_COUNTER(&SH2_HTIM); }

/**
 * @brief Run a dummy SPI operation for dummy SPI SCLK during initialization.
 */
static void spi_dummy_op(void) {
  uint8_t dummy_tx[1];
  uint8_t dummy_rx[1];
  memset(dummy_tx, 0xAA, sizeof(dummy_tx));

  // SPI clock pull-down required for SH2 communications initialization.
  // Blocking transmission with reduced timeout.
  HAL_SPI_TransmitReceive(&SH2_HSPI, dummy_tx, dummy_rx, sizeof(dummy_tx), 2);
}

/**
 * @brief SH2 delay in us.
 */
void delay_us(const uint32_t delay) {
  volatile uint32_t now = time_now_us();
  const uint32_t start = now;
  while ((now - start) < delay) {
    now = time_now_us();
  }
}

/**
 * @brief SH2 reset delay in us.
 */
void reset_delay_us(const uint32_t delay) {
  volatile uint32_t now = time_now_us();
  const uint32_t start = now;
  while (((now - start) < delay) && (in_reset)) {
    now = time_now_us();
  }
}

/**
 * @brief Attempt to start SPI operation.
 */
static void spi_activate(void) {
  // SPI is idle and not processing Rx buffer.
  if ((spi_state == SPI_IDLE) && (rx_buf_len == 0)) {
    // Ready to transmit to MCU on Rx.
    if (intn_seen_rx_ready) {
      intn_seen_rx_ready = false;

      // Ready to receive.
      cs_write_pin(GPIO_PIN_RESET);

      // Ready to transmit if buffer is filled.
      if (tx_buf_len > 0) {
        spi_state = SPI_WRITE;

        // Start operation to write (and, incidentally, read).
        HAL_SPI_TransmitReceive_IT(&SH2_HSPI, tx_buffer, rx_buffer, tx_buf_len);

        // Deassert wake.
        ps0_wake_write_pin(GPIO_PIN_SET);

        // Ready to read incoming SHTP header.
      } else {
        spi_state = SPI_RD_HDR;

        // Start SPI operation to read header (writing zeros).
        HAL_SPI_TransmitReceive_IT(&SH2_HSPI, (uint8_t *)tx_zeros, rx_buffer,
                                   READ_LEN);
      }
    }
  }
}

/**
 * @brief Handle the end of a SPI operation.
 */
static void spi_completed(void) {
  // Get length of payload available.
  uint16_t rx_payload_len = (rx_buffer[0] + (rx_buffer[1] << 8)) & ~0x8000;

  // Truncate that to max len we can read.
  if (rx_payload_len > sizeof(rx_buffer)) {
    rx_payload_len = sizeof(rx_buffer);
  }

  // SPI dummy operation, transition to idle.
  if (spi_state == SPI_DUMMY) {
    spi_state = SPI_IDLE;

    // Read a header.
  } else if (spi_state == SPI_RD_HDR) {
    // More to read in the received payload.
    if (rx_payload_len > READ_LEN) {
      // Transition to RD_BODY state.
      spi_state = SPI_RD_BODY;

      // Start a read operation for the remaining length.
      // At this point the first READ_LEN bytes have already been read.
      HAL_SPI_TransmitReceive_IT(&SH2_HSPI, (uint8_t *)tx_zeros,
                                 rx_buffer + READ_LEN,
                                 rx_payload_len - READ_LEN);

      // No SHTP payload was received, this operation is done.
    } else {
      // Transmissions complete, deassert CS
      cs_write_pin(GPIO_PIN_SET);

      // Rx buffer is empty now.
      rx_buf_len = 0;

      // Transition to idle state and activate the next operation, if any.
      spi_state = SPI_IDLE;
      spi_activate();
    }

    // Completed the read or write of a payload, deassert CS.
  } else if (spi_state == SPI_RD_BODY) {
    // Transmissions complete, deassert CS
    cs_write_pin(GPIO_PIN_SET);

    // Check len of data read and set rxBufLen.
    rx_buf_len = rx_payload_len;

    // Transition to idle state and activate the next operation, if any.
    spi_state = SPI_IDLE;
    spi_activate();

    // Completed the read or write of a payload, deassert CSN.
  } else if (spi_state == SPI_WRITE) {
    // Transmissions complete, deassert CS
    cs_write_pin(GPIO_PIN_SET);

    // Since operation was a write, transaction was for txBufLen bytes. Thus,
    // the received data len is, at a maximum, txBufLen.
    rx_buf_len = (tx_buf_len < rx_payload_len) ? tx_buf_len : rx_payload_len;

    // Tx buffer is empty now.
    tx_buf_len = 0;

    // Transition to idle state and activate the next operation, if any.
    spi_state = SPI_IDLE;
    spi_activate();
  }
}

/** User implementations of STM32 NVIC HAL (overwriting HAL). *****************/

/**
 * @brief STM32 HAL HAL_GPIO_EXTI_Callback(...) callback user implementation.
 */
void HAL_GPIO_EXTI_Callback(uint16_t n) {
  if (n == SH2_INTN_PIN) {
    rx_timestamp_us = time_now_us();

    in_reset = false;
    intn_seen_rx_ready = true;

    // Start read, if possible.
    spi_activate();
  }
}

/**
 * @brief STM32 HAL HAL_SPI_TxRxCpltCallback(...) callback user implementation.
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (hspi == &SH2_HSPI) {
    if (is_open) {
      spi_completed();
    }
  }
}

/** Abstracted STM32 HAL functions for SH2. ***********************************/

static int sh2_spi_hal_open(sh2_Hal_t *self) {
  (void)self; // Unused.

  const int retval = SH2_OK;

  // Ensure another instance is not already open.
  if (is_open) {
    return SH2_ERR;
  }
  is_open = true; // Define open instance.

  // Enable hardware (timer).
  __HAL_TIM_ENABLE(&SH2_HTIM);
  HAL_TIM_Base_Init(&SH2_HTIM);
  HAL_TIM_Base_Start(&SH2_HTIM);

  // Initialize pin states.
  rstn_write_pin(GPIO_PIN_RESET); // Hold in reset.
  cs_write_pin(GPIO_PIN_SET);     // De-assert CS.

  // Clear rx, tx buffers.
  rx_buf_len = 0;
  tx_buf_len = 0;
  intn_seen_rx_ready = false;

  in_reset = true; // Will change back to false when INTN serviced.

  // Run dummy SPI operation.
  spi_state = SPI_DUMMY;
  spi_dummy_op();
  spi_state = SPI_IDLE;

  // Delay for RESET_DELAY_US to ensure reset takes effect.
  delay_us(RESET_DELAY_US);

  // Bring out of reset and start.
  ps0_wake_write_pin(GPIO_PIN_SET); // Start in SPI.
  // PS0 = 1, PS1 = 1 for SPI, PS1 can be done in hardware.
  rstn_write_pin(GPIO_PIN_SET); // Exit reset.

  enable_interrupts();

  // Wait for INTN signal via EXTI.
  reset_delay_us(START_DELAY_US);

  return retval;
}

static void sh2_spi_hal_close(sh2_Hal_t *self) {
  (void)self; // Unused.

  disable_interrupts();

  // Set state machine to INIT state.
  spi_state = SPI_INIT;

  // Pull reset low (reset).
  rstn_write_pin(GPIO_PIN_RESET);

  // Pull CS high (no longer calling CS).
  cs_write_pin(GPIO_PIN_SET);

  // Disable the timer.
  __HAL_TIM_DISABLE(&SH2_HTIM);

  // No longer open.
  is_open = false;
}

static int sh2_spi_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len,
                            uint32_t *t) {
  (void)self; // Unused.

  int return_val = 0;

  // Received data available...
  if (rx_buf_len > 0) {
    // And if the data will fit in this buffer...
    if (len >= rx_buf_len) {
      // Copy data to the client buffer.
      memcpy(pBuffer, rx_buffer, rx_buf_len);
      return_val = (int)rx_buf_len;

      // Set timestamp of the data.
      *t = rx_timestamp_us;

      // Clear rx_buffer so we can receive again.
      rx_buf_len = 0;
    } else {
      // Discard what was read and return error because buffer was too small.
      return_val = SH2_ERR_BAD_PARAM;
      rx_buf_len = 0;
    }
    // rx_buffer is now empty, activate SPI to send previously blocked writes.
    disable_interrupts();
    spi_activate();
    enable_interrupts();
  }

  return return_val;
}

static int sh2_spi_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len) {
  int return_val = SH2_OK;

  // Validate parameters.
  if (self == 0 || len > sizeof(tx_buffer) || (len > 0 && pBuffer == 0)) {
    return SH2_ERR_BAD_PARAM;
  }

  // If tx buffer is not empty, return 0.
  if (tx_buf_len != 0) {
    return 0;
  }

  // Copy data to tx buffer.
  memcpy(tx_buffer, pBuffer, len);
  tx_buf_len = len;
  return_val = (int)len;

  disable_interrupts();
  ps0_wake_write_pin(GPIO_PIN_RESET);
  enable_interrupts();

  return return_val;
}

static uint32_t sh2_spi_hal_get_time_us(sh2_Hal_t *self) {
  (void)self; // Unused.

  return time_now_us();
}

sh2_Hal_t *sh2_hal_init(void) {
  sh2Hal.open = sh2_spi_hal_open;
  sh2Hal.close = sh2_spi_hal_close;
  sh2Hal.read = sh2_spi_hal_read;
  sh2Hal.write = sh2_spi_hal_write;
  sh2Hal.getTimeUs = sh2_spi_hal_get_time_us;

  return &sh2Hal;
}
