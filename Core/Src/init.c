/*******************************************************************************
 * @file init.c
 * @brief Centralized init logic running in main.c.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "init.h"
#include "bmp390_runner.h"
#include "bno085_runner.h"
#include "can.h"
#include "diagnostics.h"
#include "rtc.h"
#include "runcam_hal_uart.h"
#include "scheduler.h"
#include "sd.h"
#include "telemetry.h"
#include "ublox_hal_uart.h"
#include "ws2812b_hal_pwm.h"
#include "xbee_api_hal_uart.h"

#include "configuration.h"

/** Private variables. ********************************************************/

FRESULT file_result = FR_NOT_READY;
FATFS SDFatFs; // Not following snake case conventions here, see below.
// TODO: Still evaluating if fatfs.c/h implementation of `FATFS SDFatFs;` should
//  be used instead.

uint8_t xbee_sensor_data_transmit_index = 0;
uint8_t can_sensor_data_transmit_index = 0;

/** Private functions. ********************************************************/

void micro_sd_init(void) {
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

void micro_sd_deinit() { sdio_unmount_sd(&file_result, &SDFatFs); }

void transmit_sensor_data(char *data) {
  xbee_send(XBEE_DESTINATION_64, XBEE_DESTINATION_16, (const uint8_t *)data,
            strlen(data), 0);
}

/**
 * @brief Sequential sensor data transmission manager.
 */
void sequential_transmit_sensor_data(void) {
  char data[256];

  // Reset index if out of bounds.
  if (xbee_sensor_data_transmit_index < 0 ||
      xbee_sensor_data_transmit_index > 7) {
    xbee_sensor_data_transmit_index = 0;
  }

  // TODO: Implement centralized time metric/system and diagnostics.
  switch (xbee_sensor_data_transmit_index) {
  case 0:
    sprintf(data, "temp=%f,baro=%f,f=%u", bmp390_temperature, bmp390_pressure,
            bmp390_fault_count);
    break;
  case 1:
    sprintf(data, "w=%f,i=%f,j=%f,k=%f,f=%u", bno085_quaternion_real,
            bno085_quaternion_i, bno085_quaternion_j, bno085_quaternion_k,
            bno085_fault_count);
    break;
  case 2:
    sprintf(data, "accuracy_rad=%f,accuracy_deg=%f",
            bno085_quaternion_accuracy_rad, bno085_quaternion_accuracy_deg);
    break;
  case 3:
    sprintf(data, "gyro_x=%f,gyro_y=%f,gyro_z=%f", bno085_gyro_x, bno085_gyro_y,
            bno085_gyro_z);
    break;
  case 4:
    sprintf(data, "accel_x=%f,accel_y=%f,accel_z=%f", bno085_accel_x,
            bno085_accel_y, bno085_accel_z);
    break;
  case 5:
    sprintf(data, "lin_accel_x=%f,lin_accel_y=%f,lin_accel_z=%f",
            bno085_lin_accel_x, bno085_lin_accel_y, bno085_lin_accel_z);
    break;
  case 6:
    sprintf(data, "gravity_x=%f,gravity_y=%f,gravity_z=%f", bno085_gravity_x,
            bno085_gravity_y, bno085_gravity_z);
    break;
  case 7:
    if ((gps_data.lat_dir == 'N') || (gps_data.lat_dir == 'S')) {
      // If direction data is not empty, transmit as expected.
      sprintf(data, "altitude=%f,lat=%f_%c,long=%f_%c", gps_data.altitude,
              gps_data.latitude, gps_data.lat_dir, gps_data.longitude,
              gps_data.lon_dir);
    } else {
      // If direction data is empty (error/initializing), transmit zeros.
      sprintf(data, "altitude=%f,lat=%f_%c,long=%f_%c", gps_data.altitude,
              gps_data.latitude, '0', gps_data.longitude, '0');
    }
    break;
  default:
    xbee_sensor_data_transmit_index = 0;
    break; // Unknown index.
  }

  // Transmit the data after forming the string.
  transmit_sensor_data(data);

  // Increment the index and wrap around.
  xbee_sensor_data_transmit_index = (xbee_sensor_data_transmit_index + 1) % 8;
}

void sequential_can_transmit(void) {
  // Reset index if out of bounds.
  if (can_sensor_data_transmit_index < 0 ||
      can_sensor_data_transmit_index > 9) {
    can_sensor_data_transmit_index = 0;
  }

  switch (can_sensor_data_transmit_index) {
  case 0:
    can_tx_state();
    break;
  case 1:
    can_tx_barometric();
    break;
  case 2:
    can_tx_gps1();
    break;
  case 3:
    can_tx_gps2();
    break;
  case 4:
    can_tx_gps3();
    break;
  case 5:
    can_tx_imu1();
    break;
  case 6:
    can_tx_imu2();
    break;
  case 7:
    can_tx_imu3();
    break;
  case 8:
    can_tx_imu4();
    break;
  case 9:
    can_tx_imu5();
    break;
  default:
    can_sensor_data_transmit_index = 0;
    break; // Unknown index.
  }

  // Increment the index and wrap around.
  can_sensor_data_transmit_index = (can_sensor_data_transmit_index + 1) % 10;
}

/** Public functions. *********************************************************/

void nerve_init(void) {
  // Low level peripherals.
  can_init();

  // On-board miscellaneous components.
  ws2812b_init();
  for (uint8_t led_i = 0; led_i < LED_COUNT; led_i++) {
    ws2812b_set_colour(led_i, 4, 1, 1); // Very dim purple.
  }
  ws2812b_update();

  // On-board NVM.
  micro_sd_init();

  // Radio communications.
  xbee_reset();
  xbee_init();

  // Sensors.
  // ublox_reset(); // Commented out, preserve GPS lock data.
  // The 3.3 V backup cell powering the RTC and u-blox ephemeris RAM.
  ublox_init();
  bmp390_init();
  bno085_reset();
  bno085_init();

  // Camera filming.
  // TODO: runcam_power_button() toggle may be required, add status check.
  runcam_start_recording();

  // Scheduler.
  scheduler_init(); // Initialize scheduler.
  scheduler_add_task(bmp390_get_data, 10);
  scheduler_add_task(sequential_transmit_sensor_data, 50);

#ifndef NERVE_DEBUG_FULL_CAN_TELEMETRY
  scheduler_add_task(sequential_can_transmit, 10);
#endif
}
