/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// Sensor driver for B-U585I-IOT02A

#include "sensor_drv.h"
#include "sensor_drv_hw.h"

#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_motion_sensors.h"


// Temperature Sensor

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

static uint32_t tempSensorReadSample (uint32_t num_samples, void *buf) {
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

static uint32_t tempSensorGetOverflow (void) {
  return 0U;
}

sensorDrvHW_t SensorDrvHW_0 = {
  tempSensorEnable, tempSensorDisable, tempSensorReadSample, tempSensorGetOverflow, NULL
};


// Humidity Sensor

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
static uint32_t humSensorReadSample (uint32_t num_samples, void *buf) {
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

static uint32_t humSensorGetOverflow (void) {
  return 0U;
}

sensorDrvHW_t SensorDrvHW_1 = {
  humSensorEnable, humSensorDisable, humSensorReadSample, humSensorGetOverflow, NULL
};


// Pressure Sensor
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

static uint32_t pressSensorReadSample (uint32_t num_samples, void *buf) {
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

static uint32_t pressSensorGetOverflow (void) {
  return 0U;
}

sensorDrvHW_t SensorDrvHW_2 = {
  pressSensorEnable, pressSensorDisable, pressSensorReadSample, pressSensorGetOverflow, NULL
};


// Accelerometer
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

static uint32_t accSensorReadSample (uint32_t num_samples, void *buf) {
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

static uint32_t accSensorGetOverflow (void) {
  return 0U;
}

sensorDrvHW_t SensorDrvHW_3 = {
  accSensorEnable, accSensorDisable, accSensorReadSample, accSensorGetOverflow, NULL
};


// Gyroscope
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

static uint32_t gyroSensorReadSample (uint32_t num_samples, void *buf) {
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

static uint32_t gyroSensorGetOverflow (void) {
  return 0U;
}

sensorDrvHW_t SensorDrvHW_4 = {
  gyroSensorEnable, gyroSensorDisable, gyroSensorReadSample, gyroSensorGetOverflow, NULL
};


// Magnetometer

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

static uint32_t magSensorReadSample (uint32_t num_samples, void *buf) {
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

static uint32_t magSensorGetOverflow (void) {
  return 0U;
}

sensorDrvHW_t SensorDrvHW_5 = {
  magSensorEnable, magSensorDisable, magSensorReadSample, magSensorGetOverflow, NULL
};
