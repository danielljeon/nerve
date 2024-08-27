/*******************************************************************************
 * @file bno085_runner.c
 * @brief BNO085 SH2 HAL runner.
 *******************************************************************************
 * @note
 * Developed using https://github.com/ceva-dsp/sh2-demo-nucleo as reference.
 *******************************************************************************
 */

#include "bno085_runner.h"

// Private values.
sh2_ProductIds_t prod_ids;
sh2_Hal_t *sh2_hal_instance = 0;
bool reset_occurred = false;

/**
 * @brief Configure periodic reports.
 */
static void start_reports() {
  // Each entry of sensorConfig[] represents one sensor to be configured.
  static const struct {
    int sensorId;
    sh2_SensorConfig_t config;
  } sensorConfig[] = {
      // Game Rotation Vector, 100Hz.
      {SH2_GAME_ROTATION_VECTOR, {.reportInterval_us = 10000}},

      // Stability Detector, 100 Hz, changeSensitivityEnabled.
      // {SH2_STABILITY_DETECTOR, {.reportInterval_us = 10000,
      // .changeSensitivityEnabled = true}},

      // Raw accel, 100 Hz.
      // {SH2_RAW_ACCELEROMETER, {.reportInterval_us = 10000}},

      // Raw gyroscope, 100 Hz.
      // {SH2_RAW_GYROSCOPE, {.reportInterval_us = 10000}},

      // Rotation Vector, 100 Hz.
      // {SH2_ROTATION_VECTOR, {.reportInterval_us = 10000}},

      // Gyro Integrated Rotation Vector, 100 Hz.
      // {SH2_GYRO_INTEGRATED_RV, {.reportInterval_us = 10000}},

      // Motion requests for Interactive Zero Reference Offset cal.
      // {SH2_IZRO_MOTION_REQUEST, {.reportInterval_us = 10000}},

      // Shake detector.
      // {SH2_SHAKE_DETECTOR, {.reportInterval_us = 10000}},
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
    // TODO: IMPLEMENT EVENT HANDLER.
    // printf("EventHandler  id:SHTP, %d\n", pEvent->shtpEvent);

  } else if (pEvent->eventId == SH2_GET_FEATURE_RESP) {
    // TODO: IMPLEMENT EVENT HANDLER.
    // printf("EventHandler Sensor Config, %d\n",
    // pEvent->sh2SensorConfigResp.sensorId);

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

/**
 * @brief Initialize BNO085 with SH2 HAL.
 */
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

  // Start the flow of sensor reports.
  start_reports();
}
