/*******************************************************************************
 * @file sd.h
 * @brief SD: Manages SDIO in 4-bit mode for microSD operations.
 *******************************************************************************
 */

#ifndef NERVE__SD_H
#define NERVE__SD_H

/** Includes. *****************************************************************/

#include "diskio.h"
#include "fatfs.h"
#include "fatfs_platform.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "ffconf.h"
#include "integer.h"
#include "main.h"
#include "stdbool.h"
#include <stdio.h>
#include <string.h>

/** STM32 port and pin configs. ***********************************************/

extern SD_HandleTypeDef hsd;

// SDIO.
#define SDIO_SD_DETECT_PORT GPIOC
#define SDIO_SD_DETECT_PIN GPIO_PIN_4
#define SDIO_HSD hsd

/** Public variables. *********************************************************/

extern volatile int sd_write_counter;
extern Disk_drvTypeDef disk;

/** Public functions. *********************************************************/

/**
 * @breif Determine if SD card is detected for SD implementation.
 *
 * @return Result SDIO card detect status.
 * @retval == 0 -> Success, card is detected.
 * @retval == 1 -> Failure, card is not detected.
 *
 * @note SDIO card detect is typically active high, but STM32's FATFS platform
 * BSP SDIO_Detect implementation uses active low. Thus, active low means a card
 * is detected and vice versa.
 */
uint8_t sdio_card_detected();

/**
 * @brief Mounts the SD card filesystem.
 *
 * @param file_result Pointer to the result variable.
 * @param SDFatFs Pointer to the FATFS object.
 */
void sdio_mount_sd(FRESULT *file_result, FATFS *SDFatFs);

/**
 * @brief Unmounts the SD card filesystem.
 *
 * @param file_result Pointer to the result variable.
 * @param SDFatFs Pointer to the FATFS object.
 */
void sdio_unmount_sd(FRESULT *file_result, FATFS *SDFatFs);

/**
 * @brief Opens a file on the SD card in write mode.
 *
 * @param file_result Pointer to the result variable.
 * @param file_name Name of the file to open or create on the SD card.
 */
void sdio_open_write_mode_sd_file(FRESULT *file_result, char *file_name);

/**
 * @brief Write buffer to the current open file on the SD card.
 *
 * @param file_result Pointer to the result variable.
 * @param data_buffer Pointer to the data buffer.
 * @param data_buffer_length Number of bytes to be written.
 * @param bytes_written Pointer to number of bytes written (error check).
 */
void sdio_write_sd_file(FRESULT *file_result, void *data_buffer,
                        UINT data_buffer_length, UINT *bytes_written);

/**
 * @brief Closes the current open file on the SD card.
 *
 * @param file_result Pointer to the result variable.
 */
void sdio_close_sd_card_file(FRESULT *file_result);

/**
 * @brief Adjust or sets file name for the SD card file operations.
 *
 * @param sd_file_name Character pointer to the file name string.
 */
void sdio_set_file_name(char *sd_file_name);

#endif
