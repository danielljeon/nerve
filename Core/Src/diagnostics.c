/*******************************************************************************
 * @file diagnostics.c
 * @brief Centralized high level diagnostics module.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "diagnostics.h"

/** Public variables. *********************************************************/

uint8_t can_fault_count = 0;
uint8_t bmp390_fault_count = 0;
uint8_t bno085_fault_count = 0;
uint8_t gps_fault_count = 0;

/** Public functions. *********************************************************/

void can_fault(void) { can_fault_count++; }

void bmp390_fault(void) { bmp390_fault_count++; }

void bno085_fault(void) { bno085_fault_count++; }

void gps_fault(void) { gps_fault_count++; }
