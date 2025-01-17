/*******************************************************************************
 * @file pid.h
 * @brief PID: Proportional Integral Differential controller module.
 *******************************************************************************
 * @note:
 * Based on Phil’s Lab "PID Controller Implementation in Software - Phil's Lab
 * #6" (https://youtu.be/zOByx3Izf5U), GitHub: https://github.com/pms67/PID.
 *******************************************************************************
 */

#ifndef NERVE__PID_H
#define NERVE__PID_H

/** Public structs. ***********************************************************/

typedef struct {
  // Controller gains.
  float k_p;
  float k_i;
  float k_d;

  // Derivative low-pass filter time constant.
  float tau;

  // Output limits.
  float output_min;
  float output_max;

  // Integrator limits.
  float integral_min;
  float integral_max;

  // Sample time (in seconds).
  float T;

  // Controller "memory".
  float integrator;
  float prev_error; // Required for integrator.
  float differentiator;
  float prev_measurement; // Required for differentiator.

  // Controller output.
  float out;

} pid_controller_t;

/** Public functions. *********************************************************/

void pid_init(pid_controller_t *pid);
float pid_update(pid_controller_t *pid, float set_point, float measurement);

#endif
