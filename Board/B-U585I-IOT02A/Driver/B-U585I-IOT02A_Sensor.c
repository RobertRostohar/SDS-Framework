/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * Sensor driver for B-U585I-IOT02A
 */
#include "cmsis_os2.h"
#include "B-U585I-IOT02A_Sensor.h"

/// Temperature Sensor
static int32_t tempSensorEnable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_ENV_SENSOR_Enable(0, ENV_TEMPERATURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static int32_t tempSensorDisable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_ENV_SENSOR_Disable(0, ENV_TEMPERATURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static uint32_t tempSensorReadSample (void *buf, uint32_t num_samples) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;

  (void)num_samples;

  ret = HTS221_TEMP_Get_DRDY_Status(Env_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_ENV_SENSOR_GetValue(0, ENV_TEMPERATURE, (float *)buf) == BSP_ERROR_NONE) {
      num = 1U;
    }
  }
  return num;
}

/// Humidity Sensor
static int32_t humSensorEnable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_ENV_SENSOR_Enable(0, ENV_HUMIDITY) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static int32_t humSensorDisable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_ENV_SENSOR_Disable(0, ENV_HUMIDITY) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static uint32_t humSensorReadSample (void *buf, uint32_t num_samples) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;

  (void)num_samples;

  ret = HTS221_HUM_Get_DRDY_Status(Env_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_ENV_SENSOR_GetValue(0, ENV_HUMIDITY, (float *)buf) == BSP_ERROR_NONE) {
      num = 1U;
    }
  }
  return num;
}

/// Pressure Sensor
static int32_t pressSensorEnable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_ENV_SENSOR_Enable(1, ENV_PRESSURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static int32_t pressSensorDisable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_ENV_SENSOR_Disable(1, ENV_PRESSURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static uint32_t pressSensorReadSample (void *buf, uint32_t num_samples) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;

  (void)num_samples;

  ret = LPS22HH_PRESS_Get_DRDY_Status(Env_Sensor_CompObj[1], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_ENV_SENSOR_GetValue(1, ENV_PRESSURE, (float *)buf) == BSP_ERROR_NONE) {
      num = 1U;
    }
  }
  return num;
}

/// Accelerometer
static int32_t accSensorEnable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_MOTION_SENSOR_Enable(0, MOTION_ACCELERO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static int32_t accSensorDisable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_MOTION_SENSOR_Disable(0, MOTION_ACCELERO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static uint32_t accSensorReadSample (void *buf, uint32_t num_samples) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;

  (void)num_samples;

  ret = ISM330DHCX_ACC_Get_DRDY_Status(Motion_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_MOTION_SENSOR_GetAxesRaw(0, MOTION_ACCELERO, (BSP_MOTION_SENSOR_AxesRaw_t *)buf) == BSP_ERROR_NONE) {
      num = 1U;
    }
  }
  return num;
}

/// Gyroscope
static int32_t gyroSensorEnable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_MOTION_SENSOR_Enable(0, MOTION_GYRO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static int32_t gyroSensorDisable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_MOTION_SENSOR_Disable(0, MOTION_GYRO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static uint32_t gyroSensorReadSample (void *buf, uint32_t num_samples) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;

  (void)num_samples;

  ret = ISM330DHCX_GYRO_Get_DRDY_Status(Motion_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_MOTION_SENSOR_GetAxesRaw(0, MOTION_GYRO, (BSP_MOTION_SENSOR_AxesRaw_t *)buf) == BSP_ERROR_NONE) {
      num = 1U;
    }
  }
  return num;
}

/// Magnetometer
static int32_t magSensorEnable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_MOTION_SENSOR_Enable(1, MOTION_MAGNETO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static int32_t magSensorDisable (void) {
  int32_t ret = SENSOR_ERROR;
  if (BSP_MOTION_SENSOR_Disable(1, MOTION_MAGNETO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}
static uint32_t magSensorReadSample (void *buf, uint32_t num_samples) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;

  (void)num_samples;

  ret = IIS2MDC_MAG_Get_DRDY_Status(Motion_Sensor_CompObj[1], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_MOTION_SENSOR_GetAxesRaw(1, MOTION_MAGNETO, (BSP_MOTION_SENSOR_AxesRaw_t *)buf) == BSP_ERROR_NONE) {
      num = 1U;
    }
  }
  return num;
}

sensor_t sensorList[] = {
  {&tempCfg, {0, 0, 0}, NULL, 0, {tempSensorEnable,  tempSensorDisable,  NULL,  tempSensorReadSample}},
  {&tempCfg, {0, 0, 0}, NULL, 0, {humSensorEnable,   humSensorDisable,   NULL,  humSensorReadSample}},
  {&tempCfg, {0, 0, 0}, NULL, 0, {pressSensorEnable, pressSensorDisable, NULL,  pressSensorReadSample}},
  {&tempCfg, {0, 0, 0}, NULL, 0, {accSensorEnable,   accSensorDisable,   NULL,  accSensorReadSample}},
  {&tempCfg, {0, 0, 0}, NULL, 0, {gyroSensorEnable,  gyroSensorDisable,  NULL,  gyroSensorReadSample}},
  {&tempCfg, {0, 0, 0}, NULL, 0, {magSensorEnable,   magSensorDisable,   NULL,  magSensorReadSample}},
};
