/*******************************************************************************
 * @file bmp3_hal_i2c.c
 * @brief BMP3 driver HAL abstraction via I2C.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "bmp3_hal_i2c.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/** Private variable.
 * **********************************************************/

static uint8_t device_address; // Device I2C address.

/** Definitions. **************************************************************/

#define BMP3_SHUTTLE_ID 0xD3 // BMP3 shuttle board ID.

/** Public functions. *********************************************************/

BMP3_INTF_RET_TYPE bmp3_interface_init(struct bmp3_dev *bmp3, uint8_t intf) {
  int8_t result = BMP3_OK;

  if (bmp3 != NULL) {
    // TODO: NOTE: Bus configuration hardcoded locked to I2C here!
    if (intf == BMP3_I2C_INTF) {
      device_address = BMP3_I2C_ADDRESS;
      bmp3->read = bmp3_i2c_read;
      bmp3->write = bmp3_i2c_write;
      bmp3->intf = BMP3_I2C_INTF;
    }

    bmp3->delay_us = bmp3_delay_us;
    bmp3->intf_ptr = &device_address;

  } else {
    result = BMP3_E_NULL_PTR;
  }

  return result;
}

BMP3_INTF_RET_TYPE bmp3_i2c_read(uint8_t reg_addr, uint8_t *reg_data,
                                 uint32_t len, void *intf_ptr) {
  const uint8_t device_addr = *(uint8_t *)intf_ptr;
  (void)intf_ptr;

  const HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
      &BMP3_HI2C, device_addr << 1, reg_addr, 1, reg_data, len, HAL_MAX_DELAY);

  // Return status code.
  if (status != HAL_OK) {
    return -1;
  }
  return BMP3_INTF_RET_SUCCESS;
}

BMP3_INTF_RET_TYPE bmp3_i2c_write(uint8_t reg_addr, const uint8_t *reg_data,
                                  uint32_t len, void *intf_ptr) {
  uint8_t device_addr = *(uint8_t *)intf_ptr << 1;
  (void)intf_ptr;

  const HAL_StatusTypeDef status =
      HAL_I2C_Mem_Write(&BMP3_HI2C, device_addr, reg_addr, 1,
                        (uint8_t *)reg_data, len, HAL_MAX_DELAY);

  // Return status code.
  if (status != HAL_OK) {
    return -1;
  }
  return BMP3_INTF_RET_SUCCESS;
}

void bmp3_delay_us(uint32_t period, void *intf_ptr) {
  (void)intf_ptr;

  __HAL_TIM_SET_COUNTER(&BMP3_HTIM, 0); // Reset the counter.
  HAL_TIM_Base_Start(&BMP3_HTIM);
  while (__HAL_TIM_GET_COUNTER(&BMP3_HTIM) < period)
    ;
  HAL_TIM_Base_Stop(&BMP3_HTIM);
}

void bmp3_result_error_handler(const int8_t result) {
  switch (result) {
  case BMP3_OK:
    break;
  case BMP3_E_NULL_PTR:
    // TODO: Error handling for null pointer.
    break;
  case BMP3_E_COMM_FAIL:
    // TODO: Error handling for communication failure.
    break;
  case BMP3_E_INVALID_LEN:
    // TODO: Error handling for incorrect length parameter.
    break;
  case BMP3_E_DEV_NOT_FOUND:
    // TODO: Error handling for device not found.
    break;
  case BMP3_E_CONFIGURATION_ERR:
    // TODO: Error handling for configuration Error.
    break;
  case BMP3_W_SENSOR_NOT_ENABLED:
    // TODO: Error handling for warning when Sensor not enabled.
    break;
  case BMP3_W_INVALID_FIFO_REQ_FRAME_CNT:
    // TODO: Error handling for warning Fifo watermark level not in limit.
    break;
  default:
    // TODO: Error handling for unknown error code.
    break;
  }
}
