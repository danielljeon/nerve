/*******************************************************************************
 * @file controls_6dof.h
 * @brief General 6 DOF control systems logic (x, y, z, pitch, yaw, roll).
 *******************************************************************************
 */

#ifndef NERVE__CONTROLS_6DOF_H
#define NERVE__CONTROLS_6DOF_H

/** Includes. *****************************************************************/

#include "pid.h"

/** Public types. *************************************************************/

/**
 * @breif Defined type for PID sensor measurements.
 */
typedef struct {
  float latitude;  // Loop 1: Position, latitude.
  float longitude; // Loop 1: Position, longitude.
  float altitude;  // Loop 1: Position, altitude.

  float velocity_x; // Loop 2: Velocity, velocity_x.
  float velocity_y; // Loop 2: Velocity, velocity_y.
  float velocity_z; // Loop 2: Velocity, velocity_z.

  float attitude_pitch; // Loop 3: Attitude, attitude_pitch.
  float attitude_yaw;   // Loop 3: Attitude, attitude_yaw.
  float attitude_roll;  // Loop 3: Attitude, attitude_roll.

  float rate_pitch; // Loop 4: Rate, rate_pitch.
  float rate_yaw;   // Loop 4: Rate, rate_yaw.
  float rate_roll;  // Loop 4: Rate, rate_roll.

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
