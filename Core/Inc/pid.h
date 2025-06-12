/*******************************************************************************
 * @file pid.h
 * @brief PID: Proportional Integral Differential controller module.
 *******************************************************************************
 * @note:
 * Based on Phil's Lab "PID Controller Implementation in Software - Phil's Lab
 * #6" (https://youtu.be/zOByx3Izf5U), GitHub: https://github.com/pms67/PID.
 *******************************************************************************
 */

#ifndef NERVE__PID_H
#define NERVE__PID_H

/** Public types. *************************************************************/

typedef struct {
  float k_p; // Controller k_p gain.
  float k_i; // Controller k_i gain.
  float k_d; // Controller k_d gain.

  float tau; // Derivative low-pass filter time constant.

  float output_min; // Output minimum limit.
  float output_max; // Output maximum limit.

  float integral_min; // Integrator minimum limit.
  float integral_max; // Integrator maximum limit.

  float T; // Sample time (in seconds).

  float integrator;       // Controller integrator value.
  float prev_error;       // Previous error, required for integrator.
  float differentiator;   // Controller differentiator value.
  float prev_measurement; // Previous measurement, required for differentiator.

  float out; // Controller output.

} pid_controller_t;

/** Public functions. *********************************************************/

void pid_init(pid_controller_t *pid);
float pid_update(pid_controller_t *pid, float set_point, float measurement);

#endif
