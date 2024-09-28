/*******************************************************************************
 * @file sensors.c
 * @brief Sensor Manager: Manages sensor data using moving window filters.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "sensors.h"
#include <stdio.h>

#include "bmp390_runner.h"
#include "bno085_runner.h"
#include "xbee_api_hal_uart.h"

/** Private variables. ********************************************************/

/** Private functions. ********************************************************/

/** Public variables. *********************************************************/

/** Public functions. *********************************************************/

int8_t sensors_init() {
  //  bno085_init();
  //  bmp390_init();
  return 0;
}

void sensors_run(void) {
  //  // TODO: BNO085 IMU with sensor fusion firmware.
  //  bno085_run();

  //  // TODO: BMP390 barometric pressure and temperature sensor.
  //  bmp390_get_data();
}
