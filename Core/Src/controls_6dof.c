/*******************************************************************************
 * @file controls_6dof.c
 * @brief General 6 DOF control systems logic (x, y, z, pitch, yaw, roll).
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "controls_6dof.h"

/** Private variables. ********************************************************/

// Position controllers.
pid_controller_t pid_position_x = {0};
pid_controller_t pid_position_y = {0};
pid_controller_t pid_position_z = {0};

// Velocity controllers.
pid_controller_t pid_velocity_x = {0};
pid_controller_t pid_velocity_y = {0};
pid_controller_t pid_velocity_z = {0};

// Attitude controllers.
pid_controller_t pid_attitude_pitch = {0};
pid_controller_t pid_attitude_yaw = {0};
pid_controller_t pid_attitude_roll = {0};

// (Rotation) rate controllers.
pid_controller_t pid_rate_pitch = {0};
pid_controller_t pid_rate_yaw = {0};
pid_controller_t pid_rate_roll = {0};

// Outer loop input set points.
float commanded_position_x = 0;
float commanded_position_y = 0;
float commanded_position_z = 0;

// Inner loop input set points.
float commanded_attitude_pitch = 0;
float commanded_attitude_yaw = 0;
float commanded_pid_attitude = 0;

/** Public variables. *********************************************************/

// Sensor measurements data.
pid_measurements_t data = {};

// Actuator control set points.
float commanded_actuator_pitch = 0;
float commanded_actuator_yaw = 0;
float commanded_actuator_roll = 0;

/** Public functions. *********************************************************/

void outer_loop(void) {
  // Run position loop to obtain velocity set points.
  float commanded_velocity_x =
      pid_update(&pid_position_x, commanded_position_x, data.attitude_pitch);
  float commanded_velocity_y =
      pid_update(&pid_position_y, commanded_position_y, data.attitude_yaw);
  float commanded_velocity_z =
      pid_update(&pid_position_z, commanded_position_z, data.attitude_roll);

  // Run velocity loop to obtain actuators and attitude set points.
  commanded_attitude_pitch =
      pid_update(&pid_velocity_x, commanded_velocity_x, data.attitude_pitch);
  commanded_attitude_yaw =
      pid_update(&pid_velocity_y, commanded_velocity_y, data.attitude_yaw);
  commanded_pid_attitude =
      pid_update(&pid_velocity_z, commanded_velocity_z, data.attitude_roll);
}

void inner_loop(void) {
  // Run attitude loop to obtain rotation rate set points.
  float commanded_rate_pitch = pid_update(
      &pid_attitude_pitch, commanded_attitude_pitch, data.attitude_pitch);
  float commanded_rate_yaw =
      pid_update(&pid_attitude_yaw, commanded_attitude_yaw, data.attitude_yaw);
  float commanded_rate_roll = pid_update(
      &pid_attitude_roll, commanded_pid_attitude, data.attitude_roll);

  // Run rotation rate loop to obtain actuator commands.
  commanded_actuator_pitch =
      pid_update(&pid_rate_pitch, commanded_rate_pitch, data.rate_pitch);
  commanded_actuator_yaw =
      pid_update(&pid_rate_yaw, commanded_rate_yaw, data.rate_yaw);
  commanded_actuator_roll =
      pid_update(&pid_rate_roll, commanded_rate_roll, data.rate_roll);
}
