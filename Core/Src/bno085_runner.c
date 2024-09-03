/*******************************************************************************
 * @file bno085_runner.c
 * @brief BNO085 SH2 HAL runner.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "sh2.h"
#include "sh2_err.h"
#include "sh2_hal_spi.h"

#include "bno085_runner.h"

/** Private varaibles. ********************************************************/

sh2_ProductIds_t prod_ids;
sh2_Hal_t *sh2_hal_instance = 0;
bool reset_occurred = false;

/** Private functions. ********************************************************/

/**
 * @brief Configure periodic reports.
 *
 * @note See section `5.1 Sensor Metadata` in
 * `1000-3625 - SH-2 Reference Manual v1.4` for all possible metadata records.
 */
static void start_reports() {
  // Each entry of sensorConfig[] represents one sensor to be configured.
  static const struct {
    int sensorId;
    sh2_SensorConfig_t config;
  } sensor_config[] = {
      // Fused orientation quaternion.
      // 100 Hz.
      {SH2_ROTATION_VECTOR, {.reportInterval_us = 10000}},

      // Calibrated gyroscope data.
      // 100 Hz.
      {SH2_GYROSCOPE_CALIBRATED, {.reportInterval_us = 10000}},

      // Calibrated accelerometer data on X, Y and Z axes.
      // 100 Hz.
      {SH2_ACCELEROMETER, {.reportInterval_us = 10000}},

      // Linear acceleration minus/isolated from the gravitational component.
      // 100 Hz.
      {SH2_LINEAR_ACCELERATION, {.reportInterval_us = 10000}},

      // Basic temp at IC.
      // 1 Hz.
      {SH2_TEMPERATURE, {.reportInterval_us = 100}},
  };

  for (int n = 0; n < ARRAY_LEN(sensor_config); n++) {
    const int status = sh2_setSensorConfig(sensor_config[n].sensorId,
                                           &sensor_config[n].config);
    if (status != 0) {
      // TODO: Error handling for enable sensor fail.
    }
  }
}

/**
 * @brief Handle non-sensor events from the sensor hub.
 */
static void general_event_handler(void *cookie, sh2_AsyncEvent_t *pEvent) {
  // If we see a reset, set a flag so that sensors will be reconfigured.
  if (pEvent->eventId == SH2_RESET) {
    reset_occurred = true;

  } else if (pEvent->eventId == SH2_SHTP_EVENT) {
    // TODO: IMPLEMENT EVENT HANDLER pEvent->shtpEvent.

  } else if (pEvent->eventId == SH2_GET_FEATURE_RESP) {
    // TODO: IMPLEMENT EVENT HANDLER for pEvent->sh2SensorConfigResp.sensorId.

  } else {
    // TODO: Error handling for unknown event id.
  }
}

/**
 * @breif Handle sensor events from the sensor hub.
 */
static void sensor_event_handler(void *cookie, sh2_SensorEvent_t *pEvent) {
  // TODO: IMPLEMENT EVENT HANDLER.
}

/** Public functions. *********************************************************/

void bno085_init() {
  // Create SH2 HAL instance.
  sh2_hal_instance = sh2_hal_init();

  // Open SH2 interface (also registers non-sensor event handler.)
  const int status = sh2_open(sh2_hal_instance, general_event_handler, NULL);
  if (status != SH2_OK) {
    // TODO: Error handling for sh2_open.
  }

  // Register sensor listener.
  sh2_setSensorCallback(sensor_event_handler, NULL);

  // Reset now possible it since sensor reports will be started.
  reset_occurred = false;

  // Start the flow of sensor reports.
  start_reports();
}

void bno085_run(void) {
  uint32_t now = sh2_hal_instance->getTimeUs(sh2_hal_instance);

  if (reset_occurred) {
    // Restart the flow of sensor reports.
    reset_occurred = false;
    start_reports();
  }

  // Service the sensor hub.
  // Sensor reports and event processing handled by callbacks.
  sh2_service();
}
