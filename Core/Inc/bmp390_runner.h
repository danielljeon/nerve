/*******************************************************************************
 * @file bmp390_runner.h
 * @brief BMP390 runner: init and data reading.
 *******************************************************************************
 */

#ifndef __BMP390_RUNNER_H
#define __BMP390_RUNNER_H

/** Includes. *****************************************************************/

#include "bmp3_hal_i2c.h"

#include "bmp3.h"

/** Definitions. **************************************************************/

// Defines frame count to be requested.
// This will further be used for filtering (moving average window).
#define FIFO_FRAME_COUNT UINT8_C(10)

#define FIFO_MAX_SIZE UINT16_C(70) // BMP390 Maximum FIFO size.
// Max is 512 total.
// Technically the FIFO_MAX_SIZE can be reduced to FIFO_FRAME_COUNT * 7.
// Header:      1 byte.
// Temperature: 3 bytes.
// Pressure:    3 bytes.

/** Public functions. *********************************************************/

/**
 * @brief Initialize BMP390 with BMP3 driver.
 *
 * @return Result of BMP3 API execution status.
 * @retval 0  -> Success.
 * @retval >0 -> Warning.
 * @retval <0 -> Error.
 */
int8_t bmp390_init(void);

/**
 * @brief Get pressure and temperature data using moving average filtering.
 *
 * @param temperature: Pointer to update for temperature value.
 * @param pressure: Pointer to update for pressure value.
 *
 * @return Result of BMP3 API execution status.
 * @retval 0  -> Success.
 * @retval >0 -> Warning.
 * @retval <0 -> Error.
 */
int8_t bmp390_get_data(double *temperature, double *pressure);

#endif
