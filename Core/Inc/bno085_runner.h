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

/** Public functions. *********************************************************/

/**
 * @brief Initialize BNO085 with SH2 driver.
 */
void bno085_init(void);

void bno085_run(void);

#endif
