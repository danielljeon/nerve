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
#include "scheduler.h"

/** Public functions. *********************************************************/

void nerve_init(void);

#endif
