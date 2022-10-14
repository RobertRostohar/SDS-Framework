/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * Sensor driver
 */
#include "cmsis_os2.h"
#include <string.h>

#include "B-U585I-IOT02A_Sensor.h"

/// Sensor driver functions

/**
  Get sensor identifier.
*/
sensorId_t sensorGetId (const char *name) {
  sensor_t *sensor = NULL;
  uint32_t  i;

  if (name != NULL) {
    for (i = 0U; i < (sizeof(sensorList) / sizeof(sensor_t)); i++) {
      if (strcmp(name, sensorList[i].config->name) == 0) {
        sensor = &sensorList[i];
        break;
      }
    }
  }
  return sensor;
}

/**
  Get sensor configuration.
*/
sensorConfig_t *sensorGetConfig (sensorId_t id) {
  sensor_t *sensor = id;
  sensorConfig_t *cfg = NULL;

  if (sensor != NULL) {
    cfg = sensor->config;
  }
  return cfg;
}

/**
  Register sensor events.
*/
int32_t sensorRegisterEvents (sensorId_t id, sensorEvent_t event_cb, uint32_t event_mask) {
  (void)id;
  (void)event_cb;
  (void)event_mask;

  return SENSOR_ERROR;
}

/**
  Enable sensor.
*/
int32_t sensorEnable (sensorId_t id) {
  sensor_t *sensor = id;
  int32_t ret = SENSOR_ERROR;

  if (sensor != NULL) {
    if (sensor->f.enable() == SENSOR_OK) {
      sensor->status.active = 1U;
      ret = SENSOR_OK;
    }
  }
  return ret;
}

/**
  Disable sensor.
*/
int32_t sensorDisable (sensorId_t id) {
  int32_t ret = SENSOR_ERROR;
  sensor_t *sensor = id;

  if (sensor != NULL) {
    if (sensor->f.disable() == SENSOR_OK) {
      sensor->status.active = 0U;
      ret = SENSOR_OK;
    }
  }
  return ret;
}

/**
  Read samples from sensor.
*/
uint32_t sensorReadSamples (sensorId_t id, uint32_t num_samples, void *buf, uint32_t buf_size) {
  sensor_t *sensor = id;
  uint32_t num = 0U;

  if ((sensor != NULL) && (buf != NULL) && (buf_size != 0U) && (num_samples != 0U)) {
    if (buf_size >= (num_samples * sensor->config->sample_size)) {
      num = sensor->f.readSamples(buf, num_samples);
    }
  }
  return num;
}

/**
  Get sensor status.
*/
sensorStatus_t sensorGetStatus (sensorId_t id) {
  sensor_t *sensor = id;
  sensorStatus_t stat = {0U, 0U, 0U};

  if (sensor != NULL) {
    stat.active = sensor->status.active;
  }
  return stat;
}

/**
  Get block data.
*/
void *sensorGetBlockData (sensorId_t id) {
  (void)id;
  return NULL;
}
