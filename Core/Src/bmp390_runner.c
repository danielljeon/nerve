/*******************************************************************************
 * @file bmp390_runner.c
 * @brief BMP390 runner: init and data reading.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "bmp390_runner.h"

/** Private variables. ********************************************************/

struct bmp3_dev dev;
struct bmp3_fifo_settings fifo_settings = {0};
uint8_t fifo_data[FIFO_MAX_SIZE];
struct bmp3_fifo_data fifo = {0};

// Variables used for circular buffer/window for moving average filtering.
double temperature_window[FIFO_FRAME_COUNT] = {0};
double pressure_window[FIFO_FRAME_COUNT] = {0};
int window_index = 0;
double temperature_sum = 0;
double pressure_sum = 0;

/** Public functions. *********************************************************/

int8_t bmp390_init(void) {
  const uint16_t settings_sel = BMP3_SEL_PRESS_EN | BMP3_SEL_TEMP_EN |
                                BMP3_SEL_PRESS_OS | BMP3_SEL_TEMP_OS |
                                BMP3_SEL_ODR;
  const uint16_t settings_fifo = BMP3_SEL_FIFO_MODE | BMP3_SEL_FIFO_PRESS_EN |
                                 BMP3_SEL_FIFO_TEMP_EN | BMP3_SEL_FIFO_FULL_EN |
                                 BMP3_SEL_FIFO_DOWN_SAMPLING |
                                 BMP3_SEL_FIFO_FILTER_EN;
  struct bmp3_settings settings = {0};

  int8_t hal_init_status = bmp3_interface_init(&dev, BMP3_I2C_INTF);
  if (hal_init_status != 0) {
    bmp3_result_error_handler(hal_init_status);
  }

  int8_t bmp3_init_status = bmp3_init(&dev);
  if (bmp3_init_status != 0) {
    bmp3_result_error_handler(bmp3_init_status);
  }

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

int8_t bmp390_get_data(double *temperature, double *pressure) {
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
          double new_temp = fifo_p_t_data[index].temperature;
          double new_press = fifo_p_t_data[index].pressure;

          // Remove oldest value from running sums.
          temperature_sum -= temperature_window[window_index];
          pressure_sum -= pressure_window[window_index];

          // Update window with new value.
          temperature_window[window_index] = new_temp;
          pressure_window[window_index] = new_press;

          // Add newest value in running sums.
          temperature_sum += new_temp;
          pressure_sum += new_press;

          // Update buffer index.
          window_index = (window_index + 1) % FIFO_FRAME_COUNT;
        }

        // Calculate moving average after processing all new samples.
        *temperature = temperature_sum / FIFO_FRAME_COUNT;
        *pressure = pressure_sum / FIFO_FRAME_COUNT;
        return result;
      }
      try++;
    }
  }
  return -1;
}
