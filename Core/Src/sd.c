/*******************************************************************************
 * @file sd.h
 * @brief SD: Manages SDIO in 4-bit mode for microSD operations.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "sd.h"

volatile int sd_write_counter;

/** Public functions. *********************************************************/

uint8_t sdio_card_detected() {
  GPIO_PinState detected;
  detected = HAL_GPIO_ReadPin(SDIO_SD_DETECT_PORT, SDIO_SD_DETECT_PIN);

  if (detected == GPIO_PIN_RESET) { // Active low.
    return 1;
  }
  return 0;
}

DSTATUS sdio_disk_reinitialize(BYTE physical_drive_number) {
  disk.is_initialized[physical_drive_number] = 0;
  return RES_OK;
}

void sdio_mount_sd(FRESULT *file_result, FATFS *SDFatFs) {
  if (sdio_card_detected()) {               // Ensure card is detected.
    if (HAL_SD_Init(&SDIO_HSD) != HAL_OK) { // Initialize SDIO 1-bit.
      Error_Handler();
    }

    // Switch to SDIO 4-bit now that STM32 HAL hsd (SDIO_HSD) is initialized.
    // Check successful 4-bit configuration.
    if (HAL_SD_ConfigWideBusOperation(&SDIO_HSD, SDIO_BUS_WIDE_4B) != HAL_OK) {
      Error_Handler();
    }
    // Reinitialize to ensure SD can be mounted several times.
    sdio_disk_reinitialize(SDFatFs->drv);

    // Check mount success.
    if ((*file_result = f_mount(SDFatFs, (TCHAR const *)SDPath, 0)) != FR_OK) {
      fatfs_deinit();
      *file_result = FR_DISK_ERR;
    }
  } else {
    *file_result = FR_NOT_READY;
  }
}

void sdio_unmount_sd(FRESULT *file_result, FATFS *SDFatFs) {
  // Check if mount was successful.
  if ((*file_result = f_mount(NULL, (TCHAR const *)SDPath, 1)) != FR_OK) {
    // TODO: Error handling.
    // Error_Handler();
  }
}

void sdio_open_write_mode_sd_file(FRESULT *file_result, char *file_name) {
  // Check if file open was successful.
  if ((*file_result =
           f_open(&SDFile, file_name, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
    // TODO: Error handling.
    // Error_Handler();
  }
}

void sdio_write_sd_file(FRESULT *file_result, void *data_buffer,
                        UINT data_buffer_length, UINT *bytes_written) {
  sd_write_counter++;
  *file_result =
      f_write(&SDFile, data_buffer, data_buffer_length, bytes_written);
  if ((bytes_written == 0) || (*file_result != FR_OK)) {
    // TODO: Error handling.
    // Error_Handler();
  }
}

void sdio_close_sd_card_file(FRESULT *file_result) {
  // Check if file close was successful.
  if ((*file_result = f_close(&SDFile)) != FR_OK) {
    // TODO: Error handling.
    // Error_Handler();
  }
}

void sdio_set_file_name(char *sd_file_name) {
  puts(sd_file_name); // Set name.
}
