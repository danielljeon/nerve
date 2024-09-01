/*******************************************************************************
 * @file bmp390_runner.h
 * @brief BMP390 HAL runner.
 *******************************************************************************
 */

#ifndef __BMP390_RUNNER_H
#define __BMP390_RUNNER_H

/** Includes. *****************************************************************/

#include "bmp3_hal_i2c.h"

#include "bmp3.h"

/** Definitions. **************************************************************/

// Defines frame count requested.
// As, Pressure and Temperature are enabled in this example:
// Total byte count requested = FIFO_FRAME_COUNT * BMP3_LEN_P_AND_T_HEADER_DATA.
#define FIFO_FRAME_COUNT UINT8_C(50)

#define FIFO_MAX_SIZE UINT16_C(512) // Maximum FIFO size.

/** Public functions. *********************************************************/

/**
 * @brief Initialize BNO085 with SH2 driver.
 */
int8_t bmp390_init(void);

/**
 * @brief Load pressure and temp data.
 */
void bmp390_data(void);

#endif
