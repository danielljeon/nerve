/*******************************************************************************
 * @file scheduler.h
 * @brief Scheduler: Manages real time scheduling via clock(s).
 *******************************************************************************
 */

#ifndef __SCHEDULER_H
#define __SCHEDULER_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** Definitions. **************************************************************/

/** Public structs. ***********************************************************/

/**
 * @breif Defined type for task functions.
 */
typedef void (*TaskFunction)(void);

/**
 * @breif Structure to hold task information.
 */
typedef struct {
  TaskFunction task_function;
  uint32_t period_ms;  // Task execution period in milliseconds.
  uint32_t elapsed_ms; // Time elapsed since the task was last executed.
} Task;

/** Public functions. *********************************************************/

/**
 * Function to add tasks to the scheduler.
 *
 * @param task_function: TaskFunction to add as a task.
 * @param period_ms: Task execution period in milliseconds.
 */
void scheduler_add_task(TaskFunction task_function, uint32_t period_ms);

/**
 * @breif Scheduler run function to be called in the main loop.
 */
void scheduler_run(void);

#endif
