/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// Sensor Driver

#include <string.h>

#include "sensor_config.h"
#include "sensor_drv.h"
#include "sensor_drv_hw.h"


#ifdef SENSOR0_NAME
static sensorConfig_t SensorConfig0 = {
  SENSOR0_NAME,
  SENSOR0_SAMPLE_SIZE,
  SENSOR0_SAMPLE_INTERVAL,
  SENSOR0_DATA_THRESHOLD,
  SENSOR0_FIFO_SIZE,
  SENSOR0_BLOCK_SIZE,
  SENSOR0_BLOCK_NUM
};
#endif

#ifdef SENSOR1_NAME
static sensorConfig_t SensorConfig1 = {
  SENSOR1_NAME,
  SENSOR1_SAMPLE_SIZE,
  SENSOR1_SAMPLE_INTERVAL,
  SENSOR1_DATA_THRESHOLD,
  SENSOR1_FIFO_SIZE,
  SENSOR1_BLOCK_SIZE,
  SENSOR1_BLOCK_NUM
};
#endif

#ifdef SENSOR2_NAME
static sensorConfig_t SensorConfig2 = {
  SENSOR2_NAME,
  SENSOR2_SAMPLE_SIZE,
  SENSOR2_SAMPLE_INTERVAL,
  SENSOR2_DATA_THRESHOLD,
  SENSOR2_FIFO_SIZE,
  SENSOR2_BLOCK_SIZE,
  SENSOR2_BLOCK_NUM
};
#endif

#ifdef SENSOR3_NAME
static sensorConfig_t SensorConfig3 = {
  SENSOR3_NAME,
  SENSOR3_SAMPLE_SIZE,
  SENSOR3_SAMPLE_INTERVAL,
  SENSOR3_DATA_THRESHOLD,
  SENSOR3_FIFO_SIZE,
  SENSOR3_BLOCK_SIZE,
  SENSOR3_BLOCK_NUM
};
#endif

#ifdef SENSOR4_NAME
static sensorConfig_t SensorConfig4 = {
  SENSOR4_NAME,
  SENSOR4_SAMPLE_SIZE,
  SENSOR4_SAMPLE_INTERVAL,
  SENSOR4_DATA_THRESHOLD,
  SENSOR4_FIFO_SIZE,
  SENSOR4_BLOCK_SIZE,
  SENSOR4_BLOCK_NUM
};
#endif

#ifdef SENSOR5_NAME
static sensorConfig_t SensorConfig5 = {
  SENSOR5_NAME,
  SENSOR5_SAMPLE_SIZE,
  SENSOR5_SAMPLE_INTERVAL,
  SENSOR5_DATA_THRESHOLD,
  SENSOR5_FIFO_SIZE,
  SENSOR5_BLOCK_SIZE,
  SENSOR5_BLOCK_NUM
};
#endif

#ifdef SENSOR6_NAME
static sensorConfig_t SensorConfig6 = {
  SENSOR6_NAME,
  SENSOR6_SAMPLE_SIZE,
  SENSOR6_SAMPLE_INTERVAL,
  SENSOR6_DATA_THRESHOLD,
  SENSOR6_FIFO_SIZE,
  SENSOR6_BLOCK_SIZE,
  SENSOR6_BLOCK_NUM
};
#endif

#ifdef SENSOR7_NAME
static sensorConfig_t SensorConfig7 = {
  SENSOR7_NAME,
  SENSOR7_SAMPLE_SIZE,
  SENSOR7_SAMPLE_INTERVAL,
  SENSOR7_DATA_THRESHOLD,
  SENSOR7_FIFO_SIZE,
  SENSOR7_BLOCK_SIZE,
  SENSOR7_BLOCK_NUM
};
#endif


// Control block
typedef struct {
  sensorConfig_t *config;
  sensorDrvHW_t  *drw_hw;
  sensorStatus_t  status;
  sensorEvent_t   event_cb;
  uint32_t        event_mask;
} sensor_t;

static sensor_t Sensors[8] = {
#ifdef SENSOR0_NAME
  { &SensorConfig0, &SensorDrvHW_0, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR1_NAME
  { &SensorConfig1, &SensorDrvHW_1, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR2_NAME
  { &SensorConfig2, &SensorDrvHW_2, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR3_NAME
  { &SensorConfig3, &SensorDrvHW_3, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR4_NAME
  { &SensorConfig4, &SensorDrvHW_4, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR5_NAME
  { &SensorConfig5, &SensorDrvHW_5, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR6_NAME
  { &SensorConfig6, &SensorDrvHW_6, {0U, 0U, 0U}, NULL, 0U },
#endif
#ifdef SENSOR7_NAME
  { &SensorConfig7, &SensorDrvHW_7, {0U, 0U, 0U}, NULL, 0U },
#endif
};


// Get sensor identifier
sensorId_t sensorGetId (const char *name) {
  sensor_t *sensor = NULL;
  uint32_t n;

  if (name != NULL) {
    for (n = 0U; n < (sizeof(Sensors) / sizeof(sensor_t)); n++) {
      if (strcmp(name, Sensors[n].config->name) == 0) {
        sensor = &Sensors[n];
        break;
      }
    }
  }
  return sensor;
}

// Get sensor configuration
sensorConfig_t *sensorGetConfig (sensorId_t id) {
  sensor_t *sensor = id;
  sensorConfig_t *cfg = NULL;

  if (sensor != NULL) {
    cfg = sensor->config;
  }
  return cfg;
}

// Register sensor events
int32_t sensorRegisterEvents (sensorId_t id, sensorEvent_t event_cb, uint32_t event_mask) {
  sensor_t *sensor = id;
  int32_t ret = SENSOR_ERROR;

  if ((sensor != NULL) && (event_cb != NULL) && (event_mask != 0U)) {
    sensor->event_cb   = event_cb;
    sensor->event_mask = event_mask;
    ret = SENSOR_OK;
  }

  return ret;
}

// Enable sensor
int32_t sensorEnable (sensorId_t id) {
  sensor_t *sensor = id;
  int32_t ret = SENSOR_ERROR;

  if (sensor != NULL) {
    if (sensor->drw_hw->Enable() == SENSOR_OK) {
      sensor->status.active = 1U;
      ret = SENSOR_OK;
    }
  }
  return ret;
}

// Disable sensor
int32_t sensorDisable (sensorId_t id) {
  sensor_t *sensor = id;
  int32_t ret = SENSOR_ERROR;

  if (sensor != NULL) {
    if (sensor->drw_hw->Disable() == SENSOR_OK) {
      sensor->status.active = 0U;
      ret = SENSOR_OK;
    }
  }
  return ret;
}

// Read samples from sensor
uint32_t sensorReadSamples (sensorId_t id, uint32_t num_samples, void *buf, uint32_t buf_size) {
  sensor_t *sensor = id;
  uint32_t num = 0U;

  if ((sensor != NULL) && (num_samples != 0U) && (buf != NULL) &&
      (buf_size >= (num_samples * sensor->config->sample_size)) {
    if (sensor->drw_hw->ReadSamples != NULL) {
      num = sensor->drw_hw->ReadSamples(num_samples, buf);
    }
  }
  return num;
}

// Get sensor status
sensorStatus_t sensorGetStatus (sensorId_t id) {
  sensor_t *sensor = id;
  sensorStatus_t stat = {0U, 0U, 0U};

  if (sensor != NULL) {
    stat.active   = sensor->status.active;
    stat.overflow = sensor->drw_hw->GetOverflow();
  }
  return stat;
}

// Get block data
void *sensorGetBlockData (sensorId_t id) {
  sensor_t *sensor = id;
  void *block_data = NULL;

  if ((sensor != NULL) && (sensor->drw_hw->GetBlockData != NULL)) {
    block_data = sensor->drw_hw->GetBlockData();
  }

  return block_data;
}
