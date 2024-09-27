/*******************************************************************************
 * @file scheduler.h
 * @brief Scheduler: Manages real time scheduling via DWT.
 *******************************************************************************
 */

#ifndef __SCHEDULER_H
#define __SCHEDULER_H

/** Includes. *****************************************************************/

#include "stm32f4xx_hal.h"

/** Definitions. **************************************************************/

#define MAX_TASKS 10

/** Public structs. ***********************************************************/

/**
 * @breif Defined type for task functions.
 */
typedef void (*TaskFunction)(void);

/**
 * @breif Structure to hold task information.
 *
 * task_function: A function pointer to the task that needs to be executed.
 * period_cyc: The period of the task in terms of CPU cycles.
 *  This is calculated by converting the desired period in milliseconds to CPU
 *  cycles using the formula period_cyc = period_ms * CPU_CYCLES_PER_MS.
 * next_execution_cyc: The absolute CPU cycle count at which the task is next
 *  scheduled to run. This is used to determine when the task should be executed
 *  based on the DWT cycle counter.
 */
typedef struct {
  TaskFunction task_function;  // Pointer to the task function.
  uint32_t period_cyc;         // Task execution period in CPU cycles.
  uint32_t next_execution_cyc; // Next execution time in CPU cycles.
} Task;

/** Public functions. *********************************************************/

/**
 * @breif Initialize scheduler which utilized DWT.
 */
void scheduler_init(void);

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
