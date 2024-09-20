/*******************************************************************************
 * @file bmp390_runner.c
 * @brief BMP390 runner: init and data reading.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "bmp390_runner.h"

#include <stdio.h>

/** Public functions. *********************************************************/

struct bmp3_dev dev;
struct bmp3_fifo_settings fifo_settings = {0};
struct bmp3_fifo_data fifo = {0};

int8_t bmp390_init(void) {
  const uint16_t settings_sel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN |
                                BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS |
                                BMP3_SEL_ODR;
  const uint16_t settings_fifo = BMP3_SEL_FIFO_MODE | BMP3_SEL_FIFO_PRESS_EN |
                                 BMP3_SEL_FIFO_TEMP_EN | BMP3_SEL_FIFO_FULL_EN |
                                 BMP3_SEL_FIFO_DOWN_SAMPLING |
                                 BMP3_SEL_FIFO_FILTER_EN;
  uint8_t fifo_data[FIFO_MAX_SIZE];
  struct bmp3_settings settings = {0};

  bmp3_interface_init(&dev, BMP3_I2C_INTF);

  bmp3_init(&dev);

  // Initialize FIFO settings.
  fifo_settings.mode = BMP3_ENABLE;                       // Enable.
  fifo_settings.press_en = BMP3_ENABLE;                   // Pressure.
  fifo_settings.temp_en = BMP3_ENABLE;                    // Temperature.
  fifo_settings.filter_en = BMP3_ENABLE;                  // Filtered.
  fifo_settings.down_sampling = BMP3_FIFO_NO_SUBSAMPLING; // No down-sample.
  fifo_settings.ffull_en = BMP3_ENABLE;                   // FIFO full enable.
  fifo_settings.time_en = BMP3_ENABLE;                    // Time enable.

  // Initialize FIFO.
  fifo.buffer = fifo_data;
  fifo.req_frames = FIFO_FRAME_COUNT;

  // Initialize general settings.
  settings.press_en = BMP3_ENABLE; // Pressure.
  settings.temp_en = BMP3_ENABLE;  // Temperature.
  settings.odr_filter.press_os =
      BMP3_NO_OVERSAMPLING; // No oversample pressure output data rate (ODR).
  settings.odr_filter.temp_os =
      BMP3_NO_OVERSAMPLING;                 // No oversample temperature ODR.
  settings.odr_filter.odr = BMP3_ODR_50_HZ; // Base ORD of 50 Hz.

  bmp3_set_sensor_settings(settings_sel, &settings, &dev);

  // Set normal operation mode.
  settings.op_mode = BMP3_MODE_NORMAL;
  bmp3_set_op_mode(&settings, &dev);

  int8_t result = bmp3_set_fifo_settings(settings_fifo, &fifo_settings, &dev);
  // TODO: Error handle via bmp3_get_fifo_settings(&fifo_settings, &dev);

  return result;
}

void bmp390_data(void) {
  // BMP3 driver variables.
  uint16_t fifo_length = 0;
  struct bmp3_status status = {{0}};
  struct bmp3_data fifo_p_t_data[FIFO_MAX_SIZE];

  uint8_t try = 1;

  // Begin reading fifo full interrupt data.
  while (try <= 10) {
    int8_t result = bmp3_get_status(&status, &dev);

    if ((result == BMP3_OK) && (status.intr.fifo_full == BMP3_ENABLE)) {
      bmp3_get_fifo_length(&fifo_length, &dev);
      bmp3_get_fifo_data(&fifo, &fifo_settings, &dev);

      // Read status register again to clear FIFO Full interrupt status.
      result = bmp3_get_status(&status, &dev);

      if (result == BMP3_OK) {
        bmp3_extract_fifo_data(fifo_p_t_data, &fifo, &dev);

        for (uint8_t index = 0; index < fifo.parsed_frames; index++) {
#ifdef BMP3_FLOAT_COMPENSATION
          printf("Frame[%d]  T: %.2f deg C, P: %.2f Pa\n", index,
                 (fifo_p_t_data[index].temperature),
                 (fifo_p_t_data[index].pressure));
#else
          printf("Frame[%d]  T: %ld deg C, P: %lu Pa\n", index,
                 (long int)(int32_t)(fifo_p_t_data[index].temperature / 100),
                 (long unsigned int)(uint32_t)(fifo_p_t_data[index].pressure /
                                               100));
#endif
        }
      }
      try++;
    }
  }
}
