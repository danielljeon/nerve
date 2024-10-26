/*******************************************************************************
 * @file bno085_runner.h
 * @brief BNO085 SH2 runner: init, start reports and event handling.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

#ifndef __BNO085_RUNNER_H
#define __BNO085_RUNNER_H

/** Definitions. **************************************************************/

#define RAD_TO_DEG (180.0 / 3.14159265358)

/** Public variables. *********************************************************/

extern float bno085_quaternion_i;
extern float bno085_quaternion_j;
extern float bno085_quaternion_k;
extern float bno085_quaternion_real;
extern float bno085_quaternion_accuracy_rad;
extern float bno085_quaternion_accuracy_deg;
extern float bno085_gyro_x;
extern float bno085_gyro_y;
extern float bno085_gyro_z;
extern float bno085_accel_x;
extern float bno085_accel_y;
extern float bno085_accel_z;
extern float bno085_lin_accel_x;
extern float bno085_lin_accel_y;
extern float bno085_lin_accel_z;
extern float bno085_gravity_x;
extern float bno085_gravity_y;
extern float bno085_gravity_z;
extern float bno085_pressure;
extern float bno085_temperature;

/** Public functions. *********************************************************/

/**
 * @brief Initialize BNO085 with SH2 driver.
 */
void bno085_init(void);

void bno085_run(void);

#endif
