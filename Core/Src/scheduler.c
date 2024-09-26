/*******************************************************************************
 * @file scheduler.c
 * @brief Scheduler: Manages real time scheduling via clock(s).
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "scheduler.h"

/** Definitions. **************************************************************/

/** Private variables. ********************************************************/

/** User implementations of STM32 NVIC HAL (overwriting HAL). *****************/

/** Public functions. *********************************************************/

void scheduler_add_task(TaskFunction task_function, uint32_t period_ms) {}

void scheduler_run(void) {}
