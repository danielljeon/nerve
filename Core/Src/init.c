/*******************************************************************************
 * @file init.c
 * @brief Centralized init logic running in main.c.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "init.h"
#include "sd.h"
#include "xbee_api_hal_uart.h"

/** Definitions. **************************************************************/

#define XBEE_DESTINATION_64 0x0123456789ABCDEF
#define XBEE_DESTINATION_16 0xFFFE

/** Private variables. ********************************************************/

FRESULT file_result = FR_NOT_READY;
FATFS SDFatFs; // Not following snake case conventions here, see below.
// TODO: Still evaluating if fatfs.c/h implementation of `FATFS SDFatFs;` should
//  be used instead.

/** Private functions. ********************************************************/

void micro_sd_init() {
  UINT bytes_written;

  // Note: array lengths here are char count + 1 (null terminator).
  char file_name[8] = "log.txt";
  char data[16] = "Logging started";

  // Initialize SD card and run checks.
  sdio_mount_sd(&file_result, &SDFatFs);
  if (file_result == FR_OK) {
    sdio_set_file_name(file_name);
    sdio_open_write_mode_sd_file(&file_result, file_name);
    if (file_result == FR_OK) {
      sdio_write_sd_file(&file_result, (void *)data, strlen(data),
                         &bytes_written);
      if (file_result == FR_OK && bytes_written > 0) {
        sdio_close_sd_card_file(&file_result);
      } else {
        // TODO:Error handling for bytes not written or file_result not OK.
      }
    } else {
      // TODO:Error handling for file open fails.
    }
  } else {
    // TODO:Error handling for SD card mount fails.
  }
}

void xbee_init() {
  uint64_t destination_address = XBEE_DESTINATION_64;
  uint16_t destination_network_address = XBEE_DESTINATION_16;
  const char *message = "Nerve XBee data link init";
  uint16_t message_size = strlen(message);

  send(destination_address, destination_network_address,
       (const uint8_t *)message, message_size, 0);
}

void micro_sd_deinit() { sdio_unmount_sd(&file_result, &SDFatFs); }

/** Public functions. *********************************************************/

void nerve_init(void) {
  micro_sd_init();
  xbee_init();

  scheduler_init(); // Initialize scheduler.
}
