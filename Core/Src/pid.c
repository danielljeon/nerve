/*******************************************************************************
 * @file pid.c
 * @brief PID: Proportional Integral Differential controller module.
 *******************************************************************************
 * @note:
 * Based on Phil’s Lab "PID Controller Implementation in Software - Phil's Lab
 * #6" (https://youtu.be/zOByx3Izf5U), GitHub: https://github.com/pms67/PID.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "pid.h"

/** Public functions. *********************************************************/

void pid_init(pid_controller_t *pid) {
  // Reset controller variables.
  pid->integrator = 0.0f;
  pid->prev_error = 0.0f;

  pid->differentiator = 0.0f;
  pid->prev_measurement = 0.0f;

  pid->out = 0.0f;
}

float pid_update(pid_controller_t *pid, float set_point, float measurement) {
  // Error signal.
  float error = set_point - measurement;

  // Proportional.
  float proportional = pid->k_p * error;

  // Integral.
  pid->integrator += 0.5f * pid->k_i * pid->T * (error + pid->prev_error);

  // Anti-wind-up via integrator clamping.
  if (pid->integrator > pid->integral_max) {
    pid->integrator = pid->integral_max;
  } else if (pid->integrator < pid->integral_min) {
    pid->integrator = pid->integral_min;
  }

  // Derivative (band-limited differentiator).
  pid->differentiator =
      -(2.0f * pid->k_d * (measurement - pid->prev_measurement) +
        (2.0f * pid->tau - pid->T) * pid->differentiator) /
      (2.0f * pid->tau + pid->T);
  // Note: derivative on measurement, therefore minus sign in front of equation.

  // Compute output.
  pid->out = proportional + pid->integrator + pid->differentiator;

  // Apply output clamping.
  if (pid->out > pid->output_max) {
    pid->out = pid->output_max;
  } else if (pid->out < pid->output_min) {
    pid->out = pid->output_min;
  }

  // Store error and measurement for later use.
  pid->prev_error = error;
  pid->prev_measurement = measurement;

  // Return controller output.
  return pid->out;
}
