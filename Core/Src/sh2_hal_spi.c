/*******************************************************************************
 * @file sh2_hal_spi.c
 * @brief BNO085 SH2 HAL via SHTP SPI.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "sh2_hal_spi.h"

/** Private types. ************************************************************/

typedef enum SpiState_e {
  SPI_INIT,
  SPI_DUMMY,
  SPI_IDLE,
  SPI_RD_HDR,
  SPI_RD_BODY,
  SPI_WRITE
} Sh2SpiState_t;

/** Private variables. ********************************************************/

// Dummy tx data for SPI reads.
static const uint8_t tx_zeros[SH2_HAL_MAX_TRANSFER_IN] = {0};

// SPI bus access state machine state.
static Sh2SpiState_t spi_state = SPI_INIT;

// Timestamp
static volatile uint32_t rxTimestamp_us;

// True from time SH is put in reset until first INTN indication.
static volatile bool in_reset;

// Set true when INTN is observed, until RX operation starts.
static volatile bool intn_seen_rx_ready;

// Receive support.
static uint8_t rx_buffer[SH2_HAL_MAX_TRANSFER_IN];
static volatile uint32_t rx_buf_len;
static volatile bool rx_data_ready;

// Transmit support.
static uint8_t tx_buffer[SH2_HAL_MAX_TRANSFER_OUT];
static uint32_t tx_buf_len;

// Instances of the SPI HAL for SH2 and DFU.
static sh2_Hal_t sh2Hal;

static bool is_open = false;

/** Functions. ****************************************************************/

/**
 * @brief Set the reset pin.
 */
static void set_reset(const uint8_t state) {
  HAL_GPIO_WritePin(RSTN_PORT, RSTN_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Set the PS0/Wake pin.
 */
static void set_ps0_wake(const uint8_t state) {
  HAL_GPIO_WritePin(PS0_WAKEN_PORT, PS0_WAKEN_PIN,
                    state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Set the PS1 pin.
 */
static void set_ps1(const uint8_t state) {
  HAL_GPIO_WritePin(PS1_PORT, PS1_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Set the SPI CS pin.
 */
static void set_cs(const uint8_t state) {
  HAL_GPIO_WritePin(CSN_PORT, CSN_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/**
 * @brief Get the current time in us.
 */
static uint32_t time_now_us(void) {
  return __HAL_TIM_GET_COUNTER(&TIM_HANDLER);
}

/**
 * @brief Run a dummy SPI operation for dummy SPI SCLK during initialization.
 */
static void spi_dummy_op(void) {
  uint8_t dummy_tx[1] = {0xAA};
  uint8_t dummy_rx[1];
  HAL_SPI_TransmitReceive(&SPI_HANDLER, dummy_tx, dummy_rx, 1, 2);
}

/**
 * @brief Attempt to start a SPI operation.
 *
 * @note
 * This can be done from interrupt context or with interrupts disabled. If SPI
 * periph is not in use and there is data to send or receive, this will start a
 * SPI operation.
 */
static void spi_activate(void) {
  if ((spi_state == SPI_IDLE) && (rx_buf_len == 0)) {
    if (intn_seen_rx_ready) {
      // Reset flag that was set with INTN.
      intn_seen_rx_ready = false;

      set_cs(0);

      if (tx_buf_len > 0) {
        spi_state = SPI_WRITE;

        // Start operation to write (and, incidentally, read).
        HAL_SPI_TransmitReceive_IT(&SPI_HANDLER, tx_buffer, rx_buffer,
                                   tx_buf_len);

        // Deassert wake.
        set_ps0_wake(1);
      } else {
        spi_state = SPI_RD_HDR;

        // Start SPI operation to read header (writing zeros)
        HAL_SPI_TransmitReceive_IT(&SPI_HANDLER, (uint8_t *)tx_zeros, rx_buffer,
                                   READ_LEN);
      }
    }
  }
}

/**
 * @brief Handle the end of a SPI operation.
 *
 * @note
 * This can be done from interrupt context or with interrupts disabled.
 * Depending on spi_state, it may start a follow-up operation or transition to
 * idle. In the latter case, it will call spi_activate.
 */
static void spiCompleted(void) {
  // Get length of payload available.
  uint16_t rxLen = (rx_buffer[0] + (rx_buffer[1] << 8)) & ~0x8000;

  // Truncate that to max len we can read.
  if (rxLen > sizeof(rx_buffer)) {
    rxLen = sizeof(rx_buffer);
  }

  if (spi_state == SPI_DUMMY) {
    // SPI Dummy operation completed, transition now to idle.
    spi_state = SPI_IDLE;

  } else if (spi_state == SPI_RD_HDR) {
    // We read a header.

    if (rxLen > READ_LEN) {
      // There is more to read.

      // Transition to RD_BODY state.
      spi_state = SPI_RD_BODY;

      // Start a read operation for the remaining length. (We already read the
      // first READ_LEN bytes.)
      HAL_SPI_TransmitReceive_IT(&SPI_HANDLER, (uint8_t *)tx_zeros,
                                 rx_buffer + READ_LEN, rxLen - READ_LEN);
    } else {
      // No SHTP payload was received, this operation is done.
      set_cs(1);            // Deassert CSN.
      rx_buf_len = 0;       // No rx data available.
      spi_state = SPI_IDLE; // Back to idle state.
      spi_activate();       // Activate next operation, if any.
    }
  } else if (spi_state == SPI_RD_BODY) {
    // We completed the read or write of a payload, deassert CSN.
    set_cs(1);

    // Check len of data read and set rx_buf_len.
    rx_buf_len = rxLen;

    // Transition back to idle state.
    spi_state = SPI_IDLE;

    // Activate the next operation, if any.
    spi_activate();
  } else if (spi_state == SPI_WRITE) {
    // We completed the read or write of a payload
    // deassert CSN.
    set_cs(1);

    // Since operation was a write, transaction was for tx_buf_len bytes. So
    // received data len is, at a maximum, tx_buf_len.
    rx_buf_len = (tx_buf_len < rxLen) ? tx_buf_len : rxLen;

    // Tx buffer is empty now.
    tx_buf_len = 0;

    // transition back to idle state.
    spi_state = SPI_IDLE;

    // Activate the next operation, if any.
    spi_activate();
  }
}

/** Public functions. *********************************************************/

/**
 * @brief SH2's STM32 HAL GPIO EXTI callback HAL_GPIO_EXTI_Callback() override.
 */
void HAL_GPIO_EXTI_Callback(uint16_t n) {
  rxTimestamp_us = time_now_us();

  in_reset = false;
  intn_seen_rx_ready = true;

  // Start read, if possible.
  spi_activate();
}

/**
 * @brief SH2's required STM32 HAL SPI HAL_SPI_TxRxCpltCallback() override.
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
  if (is_open) {
    spiCompleted();
  }
}

/**
 * @brief SH2's required STM32 HAL SPI HAL_SPI_ErrorCallback() override.
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
  // Shouldn't happen.
  while (1)
    ;
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
void reset_delay_us(uint32_t delay) {
  volatile uint32_t now = time_now_us();
  uint32_t start = now;
  while (((now - start) < delay) && (in_reset)) {
    now = time_now_us();
  }
}

/** SH2 SPI Hal functions. ****************************************************/

/**
 * @brief SH2 SPI HAL open method.
 */
static int sh2_spi_hal_open(sh2_Hal_t *self) {
  const int retval = SH2_OK;

  if (is_open) {
    // Can't open if another instance is already open.
    return SH2_ERR;
  }

  is_open = true;

  // Hold in reset.
  set_reset(0);

  // Deassert CSN.
  set_cs(1);

  // Clear rx, tx buffers.
  rx_buf_len = 0;
  tx_buf_len = 0;
  rx_data_ready = false;
  intn_seen_rx_ready = false;

  in_reset = true; // Will change back to false when INTN serviced.

  // Do dummy SPI operation.
  // (First SPI op after reconfig has bad initial state of signals so this is a
  // throwaway operation. Afterward, all is well.)
  spi_state = SPI_DUMMY;
  spi_dummy_op();
  spi_state = SPI_IDLE;

  // Delay for RESET_DELAY_US to ensure reset takes effect
  delay_us(RESET_DELAY_US);

  // To boot in SHTP-SPI mode, must have PS1=1, PS0=1.
  // PS1 is set via jumper.
  // PS0 will be 1 PS1 jumper is 1 AND PS0_WAKEN sig is 1.
  // So we set PS0_WAKEN signal to 1.
  set_ps0_wake(1);
  set_ps1(1);

  // Deassert reset.
  set_reset(1);

  // Wait for INTN to be asserted
  reset_delay_us(START_DELAY_US);

  return retval;
}

/**
 * @brief SH2 SPI HAL close method.
 */
static void sh2_spi_hal_close(sh2_Hal_t *self) {
  // Set state machine to INIT state.
  spi_state = SPI_INIT;

  // Hold sensor hub in reset.
  set_reset(0);

  // Deassert CSN.
  set_cs(1);

  // Deinit SPI peripheral.
  HAL_SPI_DeInit(&SPI_HANDLER);

  // Deinit timer.
  __HAL_TIM_DISABLE(&TIM_HANDLER);

  // No longer open.
  is_open = false;
}

/**
 * @brief SH2 SPI HAL read method.
 */
static int sh2_spi_hal_read(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len,
                            uint32_t *t) {
  int retval = 0;

  // If there is received data available...
  if (rx_buf_len > 0) {

    // And if the data will fit in this buffer...
    if (len >= rx_buf_len) {
      // Copy data to the client buffer
      memcpy(pBuffer, rx_buffer, rx_buf_len);
      retval = rx_buf_len;

      // Set timestamp of that data
      *t = rxTimestamp_us;

      // Clear rx_buffer so we can receive again
      rx_buf_len = 0;

    } else {
      // Discard what was read and return error because buffer was too small.
      retval = SH2_ERR_BAD_PARAM;
      rx_buf_len = 0;
    }

    // Now that rx_buffer is empty, activate SPI processing to send any
    // potential write that was blocked.
    spi_activate();
  }

  return retval;
}

/**
 * @brief SH2 SPI HAL write method.
 */
static int sh2_spi_hal_write(sh2_Hal_t *self, uint8_t *pBuffer, unsigned len) {
  int retval = SH2_OK;

  // Validate parameters.
  if ((self == 0) || (len > sizeof(tx_buffer)) ||
      ((len > 0) && (pBuffer == 0))) {
    return SH2_ERR_BAD_PARAM;
  }

  // If tx buffer is not empty, return 0.
  if (tx_buf_len != 0) {
    return 0;
  }

  // Copy data to tx buffer.
  memcpy(tx_buffer, pBuffer, len);
  tx_buf_len = len;
  retval = len;

  // Assert Wake.
  set_ps0_wake(0);

  return retval;
}

/**
 * @brief SH2 SPI HAL impementation for get the current time in us.
 */
static uint32_t sh2_spi_hal_get_time_us(sh2_Hal_t *self) {
  return time_now_us();
}

/**
 * @brief Initialize SH2 HAL reference object.
 */
sh2_Hal_t *sh2_hal_init(void) {
  sh2Hal.open = sh2_spi_hal_open;
  sh2Hal.close = sh2_spi_hal_close;
  sh2Hal.read = sh2_spi_hal_read;
  sh2Hal.write = sh2_spi_hal_write;
  sh2Hal.getTimeUs = sh2_spi_hal_get_time_us;

  return &sh2Hal;
}
