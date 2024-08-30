/*******************************************************************************
 * @file bno085_runner.h
 * @brief BNO085 SH2 HAL runner.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

#ifndef __BNO085_RUNNER_H
#define __BNO085_RUNNER_H

/** Includes. *****************************************************************/

#include "sh2.h"
#include "sh2_hal_spi.h"

/** Public functions. *********************************************************/

void bno085_init(void);

#endif
