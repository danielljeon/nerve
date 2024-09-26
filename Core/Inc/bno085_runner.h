/*******************************************************************************
 * @file bno085_runner.h
 * @brief BNO085 SH2 runner: init, start reports and event handling.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

#ifndef __BNO085_RUNNER_H
#define __BNO085_RUNNER_H

/** Definitions. **************************************************************/

#define RAD_TO_DEG (180.0 / 3.14159265358)

/** Public functions. *********************************************************/

/**
 * @brief Initialize BNO085 with SH2 driver.
 */
void bno085_init(void);

void bno085_run(void);

#endif
