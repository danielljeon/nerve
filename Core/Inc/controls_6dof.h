/*******************************************************************************
 * @file controls_6dof.h
 * @brief General 6 DOF control systems logic (x, y, z, pitch, yaw, roll).
 *******************************************************************************
 */

#ifndef NERVE__CONTROLS_6DOF_H
#define NERVE__CONTROLS_6DOF_H

/** Includes. *****************************************************************/

#include "pid.h"

/** Public structs. ***********************************************************/

/**
 * @breif Defined type for PID sensor measurements.
 */
typedef struct {
  // Loop 1: Position.
  float latitude;
  float longitude;
  float altitude;

  // Loop 2: Velocity.
  float velocity_x;
  float velocity_y;
  float velocity_z;

  // Loop 3: Attitude.
  float attitude_pitch;
  float attitude_yaw;
  float attitude_roll;

  // Loop 4: Rate.
  float rate_pitch;
  float rate_yaw;
  float rate_roll;

} pid_measurements_t;

/** Public variables. *********************************************************/

pid_measurements_t data;

float commanded_actuator_pitch;
float commanded_actuator_yaw;
float commanded_actuator_roll;

/** Public functions. *********************************************************/

/**
 * @brief Outer control systems loop: position and velocity.
 */
void outer_loop(void);

/**
 * @brief Inner control systems loop: attitude and (rotation) rate.
 */
void inner_loop(void);

#endif
