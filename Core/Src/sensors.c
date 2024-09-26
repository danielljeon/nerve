/*******************************************************************************
 * @file sensors.c
 * @brief Sensor Manager: Manages sensor data using moving window filters.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "sensors.h"

#include "bmp390_runner.h"
#include "bno085_runner.h"

/** Private variables. ********************************************************/

/** Private functions. ********************************************************/

/** Public variables. *********************************************************/

/** Public functions. *********************************************************/

int8_t sensors_init() {
  //  bno085_init();  // TODO: FIX INIT FUNCTIONS!!!
  //  bmp390_init();
  return 0;
}

int8_t sensors_run() {
  //  // TODO: BNO085 IMU with sensor fusion firmware.
  //  bno085_run();

  //  // TODO: BMP390 barometric pressure and temperature sensor.
  //  int8_t bmp390_data_status;
  //  double temperature;
  //  double pressure;
  //  bmp390_data_status = bmp390_get_data(&temperature, &pressure);

  return 0;
}
