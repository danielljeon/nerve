/*******************************************************************************
 * @file sensors.h
 * @brief Sensor Manager: Manages sensor data using moving window filters.
 *******************************************************************************
 */

#ifndef __SENSOR_MANAGER_H
#define __SENSOR_MANAGER_H

/** Includes. *****************************************************************/

#include <stdbool.h>
#include <stdint.h>

/** Definitions. **************************************************************/

/** Public structs. ***********************************************************/

/** Public variables. *********************************************************/

/** Public functions. *********************************************************/

int8_t sensors_init();

int8_t sensors_run();

#endif
