/*******************************************************************************
 * @file sensors.c
 * @brief Sensor Manager: Manages sensor data using moving window filters.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include <stdio.h>
#include <string.h>

#include "bmp390_runner.h"
#include "bno085_runner.h"
#include "sensors.h"
#include "xbee_api_hal_uart.h"

/** Private variables. ********************************************************/

/** Private functions. ********************************************************/

/** Public variables. *********************************************************/

/** Public functions. *********************************************************/

int8_t sensors_init() {
  //  bno085_init();
  bmp390_init();
  return 0;
}

void sensors_run(void) {
  //  bno085_run();
  bmp390_get_data();
}
