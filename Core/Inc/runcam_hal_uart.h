/*******************************************************************************
 * @file runcam_hal_uart.h
 * @brief RunCam functions: abstracting STM32 HAL primiary UART.
 *******************************************************************************
 */

#ifndef NERVE__RUNCAM_HAL_UART_H
#define NERVE__RUNCAM_HAL_UART_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** STM32 port and pin configs. ***********************************************/

extern UART_HandleTypeDef huart3;

// UART.
#define RUNCAM_HUART huart3

/** RunCam Device protocol commands. ******************************************/

// Request (0xCC) message bytes: Header, Command ID, Action ID.
#define RUN_CAM_POWER_BUTTON 0xCC, 0x01, 0x01
#define RUN_CAM_START_RECORDING 0xCC, 0x01, 0x03
#define RUN_CAM_STOP_RECORDING 0xCC, 0x01, 0x04
#define RUN_CAM_REQUEST_STATUS 0xCC, 0x02
#define RUN_CAM_REQUEST_FIRMWARE 0xCC, 0x03
#define RUN_CAM_REQUEST_REMAINING_TIME 0xCC, 0x04
#define RUN_CAM_REQUEST_SD_CAPACITY 0xCC, 0x05
#define RUN_CAM_REQUEST_RESOLUTION 0xCC, 0x06
#define RUN_CAM_SET_RESOLUTION 0xCC, 0x07
#define RUN_CAM_SET_DATETIME 0xCC, 0x08
#define RUN_CAM_REQUEST_TV_MODE 0xCC, 0x09
#define RUN_CAM_SET_TV_MODE 0xCC, 0x0A

/** Public functions. *********************************************************/

/**
 * @brief Sends a command over UART with CRC appended.
 *
 * @param base_cmd Pointer to the command array.
 * @param base_length Length of the base command array.
 */
void runcam_send_command(const uint8_t *base_cmd, uint8_t base_length);

/**
 * @brief Sends the UART command to simulate power button.
 */
void runcam_power_button(void);

/**
 * @brief Sends the UART command to start recording.
 */
void runcam_start_recording(void);

/**
 * @brief Sends the UART command to stop recording.
 */
void runcam_stop_recording(void);

/**
 * @brief Query the camera's current recording status.
 *
 * Camera is expected to return a status byte, which is then read over UART.
 *
 * @return Recording state or status.
 * @retval == 1 -> Recording is active.
 * @retval == 0 -> Not recording.
 * @retval > 1  -> Error code.
 */
uint8_t runcam_read_recording_status(void);

/**
 * @brief Query the camera's firmware version.
 *
 * Camera is expected to return a string or byte array containing the firmware
 * version, which is read over UART.
 *
 * @param rx_version_buf Pointer to a buffer to store the device info string.
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_read_device_info(uint8_t *rx_version_buf);

/**
 * @brief Query the remaining recording time.
 *
 * Camera is expected to return a 2-byte or 4-byte integer (depending on the
 * camera spec design) representing the available recording time in seconds.
 *
 * @return Status or time value.
 * @retval > 0 -> Remaining recording time in seconds.
 * @retval < 0 -> Errors.
 */
int32_t runcam_read_remaining_recording_time(void);

/**
 * @brief Query the SD card's total capacity and available space.
 *
 * @param total_capacity  Pointer to store the total capacity (in MB).
 * @param available_space Pointer to store the available space (in MB).
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_read_sd_card_capacity(uint32_t *total_capacity,
                                 uint32_t *available_space);

/**
 * @brief Query the current camera resolution.
 *
 * Camera is expected to return a value representing the resolution.
 *
 * @param resolution Pointer to store the current resolution.
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_read_camera_resolution(uint8_t *resolution);

/**
 * @brief Set the camera's resolution.
 *
 * @param resolution The resolution to set (e.g., 1080p, 720p).
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_set_camera_resolution(uint8_t resolution);

/**
 * @brief Set the camera's date and time.
 *
 * The date and time should be provided as a Unix timestamp (4 bytes).
 *
 * @param timestamp Unix timestamp representing the date and time to set.
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_set_camera_datetime(uint32_t timestamp);

/**
 * @brief Query the current TV mode (NTSC/PAL).
 *
 * Camera is expected to return a byte representing the current TV mode (e.g., 0
 * for NTSC, 1 for PAL).
 *
 * @param mode Pointer to store the current TV mode.
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_read_tv_mode(uint8_t *mode);

/**
 * @brief Set the camera's TV mode (NTSC/PAL).
 *
 * Mode should be provided as a byte (e.g., 0 for NTSC, 1 for PAL).
 *
 * @param mode The TV mode to set (0 for NTSC, 1 for PAL).
 *
 * @return Status.
 * @retval == 0 -> Success.
 * @retval < 0  -> Errors.
 */
int runcam_set_tv_mode(uint8_t mode);

#endif
