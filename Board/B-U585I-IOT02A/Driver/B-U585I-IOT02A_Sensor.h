/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef B_U585I_IOT02A_SENSOR_H
#define B_U585I_IOT02A_SENSOR_H

#ifdef  __cplusplus
extern "C"
{
#endif
#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_motion_sensors.h"

#include "sensor_drv.h"

/// Sensor Interface Configuration

/// Temperature
#define SENSOR_TEMP_NAME                "Temperature"
#define SENSOR_TEMP_SAMPLE_SIZE         4UL
#define SENSOR_TEMP_SAMPLE_INTERVAL     1000000             // sampling rate = 1Hz - Default value set by BSP
#define SENSOR_TEMP_DATA_THRESHOLD      4UL

/// Humidity
#define SENSOR_HUM_NAME                 "Humidity"
#define SENSOR_HUM_SAMPLE_SIZE          4UL
#define SENSOR_HUM_SAMPLE_INTERVAL      1000000             // sampling rate = 1Hz - Default value set by BSP
#define SENSOR_HUM_DATA_THRESHOLD       4UL

/// Pressure
#define SENSOR_PRESS_NAME               "Pressure"
#define SENSOR_PRESS_SAMPLE_SIZE        4UL
#define SENSOR_PRESS_SAMPLE_INTERVAL    25000               // sampling rate = 40Hz - Default value set by BSP
#define SENSOR_PRESS_DATA_THRESHOLD     4UL

/// Accelerometer
#define SENSOR_ACC_NAME                 "Accelerometer"
#define SENSOR_ACC_SAMPLE_SIZE          6UL
#define SENSOR_ACC_SAMPLE_INTERVAL      9615                // sampling rate = 104Hz - Default value set by BSP
#define SENSOR_ACC_DATA_THRESHOLD       6UL

/// Gyroscope
#define SENSOR_GYRO_NAME                "Gyroscope"
#define SENSOR_GYRO_SAMPLE_SIZE         6UL
#define SENSOR_GYRO_SAMPLE_INTERVAL     9615                // sampling rate = 104Hz - Default value set by BSP
#define SENSOR_GYRO_DATA_THRESHOLD      6UL

/// Magnetometer
#define SENSOR_MAG_NAME                 "Magnetometer"
#define SENSOR_MAG_SAMPLE_SIZE          6UL
#define SENSOR_MAG_SAMPLE_INTERVAL      10000               // sampling rate = 100Hz - Default value set by BSP
#define SENSOR_MAG_DATA_THRESHOLD       6UL

static const sensorConfig_t tempCfg  = {SENSOR_TEMP_NAME,  SENSOR_TEMP_SAMPLE_SIZE,  SENSOR_TEMP_SAMPLE_INTERVAL,  SENSOR_TEMP_DATA_THRESHOLD,  0U, 0U, 0U };
static const sensorConfig_t humCfg   = {SENSOR_HUM_NAME,   SENSOR_HUM_SAMPLE_SIZE,   SENSOR_HUM_SAMPLE_INTERVAL,   SENSOR_HUM_DATA_THRESHOLD,   0U, 0U, 0U };
static const sensorConfig_t pressCfg = {SENSOR_PRESS_NAME, SENSOR_PRESS_SAMPLE_SIZE, SENSOR_PRESS_SAMPLE_INTERVAL, SENSOR_PRESS_DATA_THRESHOLD, 0U, 0U, 0U };
static const sensorConfig_t accCfg   = {SENSOR_ACC_NAME,   SENSOR_ACC_SAMPLE_SIZE,   SENSOR_ACC_SAMPLE_INTERVAL,   SENSOR_ACC_DATA_THRESHOLD,   0U, 0U, 0U };
static const sensorConfig_t gyroCfg  = {SENSOR_GYRO_NAME,  SENSOR_GYRO_SAMPLE_SIZE,  SENSOR_GYRO_SAMPLE_INTERVAL,  SENSOR_GYRO_DATA_THRESHOLD,  0U, 0U, 0U };
static const sensorConfig_t magCfg   = {SENSOR_MAG_NAME,   SENSOR_MAG_SAMPLE_SIZE,   SENSOR_MAG_SAMPLE_INTERVAL,   SENSOR_MAG_DATA_THRESHOLD,   0U, 0U, 0U };

/// Function block
typedef const struct {
  int32_t  (*enable)      (void);
  int32_t  (*disable)     (void);
  uint32_t (*getStatus)   (void);
  uint32_t (*readSamples) (void *buf, uint32_t num_samples);
} func_t;

/// Control block
typedef struct {
  sensorConfig_t   *config;
  sensorStatus_t    status;
  sensorEvent_t     event_cb;
  uint32_t          event_mask;
  func_t            f;
} sensor_t;

/// Externals
extern void *Env_Sensor_CompObj[ENV_SENSOR_INSTANCES_NBR];
extern void *Motion_Sensor_CompObj[MOTION_SENSOR_INSTANCES_NBR];

/// Export Sensor List
extern sensor_t sensorList[6];
#ifdef  __cplusplus
}
#endif

#endif /* B_U585I_IOT02A_SENSOR_H */
