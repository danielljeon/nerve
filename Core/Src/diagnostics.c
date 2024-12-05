/*******************************************************************************
 * @file diagnostics.c
 * @brief Centralized high level diagnostics module.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "diagnostics.h"

/** Public variables. *********************************************************/

uint8_t bmp390_fault_count = 0;
uint8_t bno085_fault_count = 0;

/** Public functions. *********************************************************/

void bmp390_fault(void) { bmp390_fault_count++; }

void bno085_fault(void) { bno085_fault_count++; }
