/*******************************************************************************
 * @file telemetry.h
 * @brief General telemetry functions including CAN and XBee radio.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "bmp390_runner.h"
#include "can.h"
#include "can_nerve.h"
#include "ublox_hal_uart.h"
#include "ws2812b_hal_pwm.h"

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
