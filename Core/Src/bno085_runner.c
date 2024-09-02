/*******************************************************************************
 * @file bno085_runner.c
 * @brief BNO085 SH2 HAL runner.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

/** Includes. *****************************************************************/

#include "bno085_runner.h"

/** Private varaibles. ********************************************************/

sh2_ProductIds_t prod_ids;
sh2_Hal_t *sh2_hal_instance = 0;
bool reset_occurred = false;

/** Private functions. ********************************************************/

/**
 * @brief Configure periodic reports.
 */
static void start_reports() {
  // Each entry of sensorConfig[] represents one sensor to be configured.
  static const struct {
    int sensorId;
    sh2_SensorConfig_t config;
  } sensorConfig[] = {
      {SH2_GAME_ROTATION_VECTOR, {.reportInterval_us = 10000}},
  };

  for (int n = 0; n < ARRAY_LEN(sensorConfig); n++) {
    const int status =
        sh2_setSensorConfig(sensorConfig[n].sensorId, &sensorConfig[n].config);
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
