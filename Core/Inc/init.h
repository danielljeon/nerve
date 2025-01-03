/*******************************************************************************
 * @file init.h
 * @brief Centralized init logic running in main.c.
 *******************************************************************************
 */

#ifndef NERVE__INIT_H
#define NERVE__INIT_H

/** Includes. *****************************************************************/

#include "bmp390_runner.h"
#include "bno085_runner.h"
#include "can.h"
#include "diagnostics.h"
#include "runcam_hal_uart.h"
#include "scheduler.h"
#include "sd.h"
#include "ublox_hal_uart.h"
#include "ws2812b_hal_pwm.h"
#include "xbee_api_hal_uart.h"

/** Public functions. *********************************************************/

void nerve_init(void);

#endif
