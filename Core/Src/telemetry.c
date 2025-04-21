/*******************************************************************************
 * @file telemetry.h
 * @brief General telemetry functions including CAN and XBee radio.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "bmp390_runner.h"
#include "bno085_runner.h"
#include "can.h"
#include "can_nerve.h"
#include "ublox_hal_uart.h"

/** Public functions. *********************************************************/

void can_tx_state(void) {
  can_message_t state_msg = dbc_messages[0];
  uint32_t state_sigs[1] = {0}; // TODO: Hardcoded.
  can_send_message_raw32(&hcan1, &state_msg, state_sigs);
}

void can_tx_barometric(void) {
  can_message_t pressure_msg = dbc_messages[1];
  uint32_t pressure_sigs[3] = {0};
  const double pressure_source_sigs[3] = {bmp390_pressure, bmp390_temperature,
                                          (double)bmp390_fault_count};
  for (int i = 0; i < pressure_msg.signal_count; ++i) {
    pressure_sigs[i] =
        double_to_raw(pressure_source_sigs[i], &pressure_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &pressure_msg, pressure_sigs);
}

void can_tx_gps1(void) {
  can_message_t gps1_msg = dbc_messages[2];
  uint32_t gps1_sigs[2] = {0};
  const double gps1_source_sigs[2] = {gps_data.latitude, gps_data.longitude};
  for (int i = 0; i < gps1_msg.signal_count; ++i) {
    gps1_sigs[i] = double_to_raw(gps1_source_sigs[i], &gps1_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &gps1_msg, gps1_sigs);
}

void can_tx_gps2(void) {
  can_message_t gps2_msg = dbc_messages[3];
  uint32_t gps2_sigs[2] = {0};
  const double gps2_source_sigs[2] = {gps_data.altitude, 0}; // TODO: Hardcoded.
  for (int i = 0; i < gps2_msg.signal_count; ++i) {
    gps2_sigs[i] = double_to_raw(gps2_source_sigs[i], &gps2_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &gps2_msg, gps2_sigs);
}

void can_tx_gps3(void) {
  can_message_t gps3_msg = dbc_messages[4];
  uint32_t gps3_sigs[3] = {0};
  const uint32_t gps3_source_sigs[3] = {gps_data.fix_quality,
                                        gps_data.satellites, 0};
  for (int i = 0; i < gps3_msg.signal_count; ++i) {
    gps3_sigs[i] = uint_to_raw(gps3_source_sigs[i], &gps3_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &gps3_msg, gps3_sigs);
}

void can_tx_imu1(void) {
  can_message_t imu1_msg = dbc_messages[5];
  uint32_t imu1_sigs[4] = {0};
  const float imu1_source_sigs[4] = {bno085_quaternion_i, bno085_quaternion_j,
                                     bno085_quaternion_k,
                                     bno085_quaternion_real};
  for (int i = 0; i < imu1_msg.signal_count; ++i) {
    imu1_sigs[i] = float_to_raw(imu1_source_sigs[i], &imu1_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &imu1_msg, imu1_sigs);
}

void can_tx_imu2(void) {
  can_message_t imu2_msg = dbc_messages[6];
  uint32_t imu2_sigs[3] = {0};
  const float imu2_source_sigs[3] = {bno085_gyro_x, bno085_gyro_y,
                                     bno085_gyro_z};
  for (int i = 0; i < imu2_msg.signal_count; ++i) {
    imu2_sigs[i] = float_to_raw(imu2_source_sigs[i], &imu2_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &imu2_msg, imu2_sigs);
}

void can_tx_imu3(void) {
  can_message_t imu3_msg = dbc_messages[7];
  uint32_t imu3_sigs[3] = {0};
  const float imu3_source_sigs[3] = {bno085_accel_x, bno085_accel_y,
                                     bno085_accel_z};
  for (int i = 0; i < imu3_msg.signal_count; ++i) {
    imu3_sigs[i] = float_to_raw(imu3_source_sigs[i], &imu3_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &imu3_msg, imu3_sigs);
}

void can_tx_imu4(void) {
  can_message_t imu4_msg = dbc_messages[8];
  uint32_t imu4_sigs[3] = {0};
  const float imu4_source_sigs[3] = {bno085_lin_accel_x, bno085_lin_accel_y,
                                     bno085_lin_accel_z};
  for (int i = 0; i < imu4_msg.signal_count; ++i) {
    imu4_sigs[i] = float_to_raw(imu4_source_sigs[i], &imu4_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &imu4_msg, imu4_sigs);
}

void can_tx_imu5(void) {
  can_message_t imu5_msg = dbc_messages[9];
  uint32_t imu5_sigs[3] = {0};
  const float imu5_source_sigs[3] = {bno085_gravity_x, bno085_gravity_y,
                                     bno085_gravity_z};
  for (int i = 0; i < imu5_msg.signal_count; ++i) {
    imu5_sigs[i] = float_to_raw(imu5_source_sigs[i], &imu5_msg.signals[i]);
  }
  can_send_message_raw32(&hcan1, &imu5_msg, imu5_sigs);
}
