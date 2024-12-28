/*******************************************************************************
 * @file runcam_hal_uart.c
 * @brief RunCam functions: abstracting STM32 HAL primiary UART.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "runcam_hal_uart.h"

/** Public functions. *********************************************************/

void runcam_start_recording() {
  const uint8_t start_cmd[] = {RUN_CAM_START_RECORDING};
  HAL_UART_Transmit(&RUNCAM_HUART, start_cmd, sizeof(start_cmd), HAL_MAX_DELAY);
}

void runcam_stop_recording() {
  const uint8_t stop_cmd[] = {RUN_CAM_STOP_RECORDING};
  HAL_UART_Transmit(&RUNCAM_HUART, stop_cmd, sizeof(stop_cmd), HAL_MAX_DELAY);
}

uint8_t runcam_read_recording_status(void) {
  const uint8_t request_cmd[] = {RUN_CAM_REQUEST_STATUS};
  uint8_t status = 0xFF; // Default to error value or undefined state.

  // Send the command to request the recording status.
  if (HAL_UART_Transmit(&RUNCAM_HUART, request_cmd, sizeof(request_cmd),
                        HAL_MAX_DELAY) == HAL_OK) {
    // Receive the status byte from the camera (assuming it sends 1 byte).
    if (HAL_UART_Receive(&RUNCAM_HUART, &status, 1, HAL_MAX_DELAY) == HAL_OK) {
      return status; // 0x01 = recording, 0x00 = not recording).
    }
  }

  // If transmission or reception failed, return an error code.
  return 0xFF;
}

int runcam_read_device_info(uint8_t *rx_version_buf) {
  const uint8_t request_cmd[] = {RUN_CAM_REQUEST_FIRMWARE};

  // Send the command to request the firmware version.
  if (HAL_UART_Transmit(&RUNCAM_HUART, request_cmd, sizeof(request_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission failed.
  }

  // Receive the firmware version (assuming it is returned as a string).
  if (HAL_UART_Receive(&RUNCAM_HUART, rx_version_buf, sizeof(rx_version_buf),
                       HAL_MAX_DELAY) != HAL_OK) {
    return -2; // Reception failed.
  }
  return 0; // Success.
}

int32_t runcam_read_remaining_recording_time(void) {
  uint8_t const request_cmd[] = {RUN_CAM_REQUEST_REMAINING_TIME};
  uint8_t time_buf[4] = {0};

  // Send the command to request the remaining recording time.
  if (HAL_UART_Transmit(&RUNCAM_HUART, request_cmd, sizeof(request_cmd),
                        HAL_MAX_DELAY) == HAL_OK) {
    // Receive the 4-byte remaining time value from the camera.
    if (HAL_UART_Receive(&RUNCAM_HUART, time_buf, sizeof(time_buf),
                         HAL_MAX_DELAY) == HAL_OK) {
      // Convert the 4-byte array into a 32-bit integer. (Assuming little-endian
      // format).
      const int32_t remaining_time =
          (int32_t)time_buf[0] | (int32_t)time_buf[1] << 8 |
          (int32_t)time_buf[2] << 16 | (int32_t)time_buf[3] << 24;
      return remaining_time; // Return the remaining time in seconds.
    }
  }

  // If transmission or reception failed, return a negative error code.
  return -1;
}

int runcam_read_sd_card_capacity(uint32_t *total_capacity,
                                 uint32_t *available_space) {
  const uint8_t request_cmd[] = {RUN_CAM_REQUEST_SD_CAPACITY};
  uint8_t response_buf[8] = {0}; // 4 bytes = total, 4 bytes = available space.

  // Send the command to request SD card capacity and available space.
  if (HAL_UART_Transmit(&RUNCAM_HUART, request_cmd, sizeof(request_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission error.
  }

  // Receive the response (total and available capacity in 8 bytes).
  if (HAL_UART_Receive(&RUNCAM_HUART, response_buf, sizeof(response_buf),
                       HAL_MAX_DELAY) != HAL_OK) {
    return -2; // Reception error.
  }

  // Convert the received bytes into total capacity and available space.
  // (Assuming little-endian format).
  *total_capacity = (uint32_t)response_buf[0] | (uint32_t)response_buf[1] << 8 |
                    (uint32_t)response_buf[2] << 16 |
                    (uint32_t)response_buf[3] << 24;

  *available_space =
      (uint32_t)response_buf[4] | (uint32_t)response_buf[5] << 8 |
      (uint32_t)response_buf[6] << 16 | (uint32_t)response_buf[7] << 24;

  return 0; // Success.
}

int runcam_read_camera_resolution(uint8_t *resolution) {
  const uint8_t request_cmd[] = {RUN_CAM_REQUEST_RESOLUTION};

  // Send the command to request camera resolution.
  if (HAL_UART_Transmit(&RUNCAM_HUART, request_cmd, sizeof(request_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission error.
  }

  // Receive the resolution value (1 byte).
  if (HAL_UART_Receive(&RUNCAM_HUART, resolution, 1, HAL_MAX_DELAY) != HAL_OK) {
    return -2; // Reception error.
  }

  return 0; // Success.
}

int runcam_set_camera_resolution(const uint8_t resolution) {
  const uint8_t set_cmd[] = {RUN_CAM_SET_RESOLUTION, resolution};

  // Send the command to set the camera resolution.
  if (HAL_UART_Transmit(&RUNCAM_HUART, set_cmd, sizeof(set_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission error.
  }

  return 0; // Success.
}

int runcam_set_camera_datetime(const uint32_t timestamp) {
  const uint8_t set_cmd[] = {RUN_CAM_SET_DATETIME, (timestamp & 0xFF),
                             (timestamp >> 8 & 0xFF), (timestamp >> 16 & 0xFF),
                             (timestamp >> 24 & 0xFF)};

  // Send the command to set the camera's date and time.
  if (HAL_UART_Transmit(&RUNCAM_HUART, set_cmd, sizeof(set_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission error.
  }

  return 0; // Success.
}

int runcam_read_tv_mode(uint8_t *mode) {
  const uint8_t request_cmd[] = {RUN_CAM_REQUEST_TV_MODE};

  // Send the command to request TV mode.
  if (HAL_UART_Transmit(&RUNCAM_HUART, request_cmd, sizeof(request_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission error.
  }

  // Receive the TV mode value (1 byte, 0 = NTSC, 1 = PAL).
  if (HAL_UART_Receive(&RUNCAM_HUART, mode, 1, HAL_MAX_DELAY) != HAL_OK) {
    return -2; // Reception error.
  }

  return 0; // Success.
}

int runcam_set_tv_mode(const uint8_t mode) {
  const uint8_t set_cmd[] = {RUN_CAM_SET_TV_MODE, mode};

  // Send the command to set the TV mode.
  if (HAL_UART_Transmit(&RUNCAM_HUART, set_cmd, sizeof(set_cmd),
                        HAL_MAX_DELAY) != HAL_OK) {
    return -1; // Transmission error.
  }

  return 0; // Success.
}
