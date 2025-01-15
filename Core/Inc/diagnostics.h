/*******************************************************************************
 * @file diagnostics.h
 * @brief Centralized high level diagnostics module.
 *******************************************************************************
 */

#ifndef NERVE__DIAGNOSTICS_H
#define NERVE__DIAGNOSTICS_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** Public variables. *********************************************************/

extern uint8_t can_fault_count;
extern uint8_t bmp390_fault_count;
extern uint8_t bno085_fault_count;

/** Public functions. *********************************************************/

void can_fault(void);

void bmp390_fault(void);

void bno085_fault(void);

void gps_fault(void);

#endif
