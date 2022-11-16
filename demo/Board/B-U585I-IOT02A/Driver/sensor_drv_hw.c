/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// Sensor driver for B-U585I-IOT02A

#include "sensor_drv.h"
#include "sensor_drv_hw.h"

#include "b_u585i_iot02a_env_sensors.h"
#include "b_u585i_iot02a_motion_sensors.h"


// Temperature Sensor

static int32_t TemperatureSensor_Enable (void) {
  int32_t ret = SENSOR_ERROR;
  float   value;

  if (BSP_ENV_SENSOR_Enable(0, ENV_TEMPERATURE) == BSP_ERROR_NONE) {
    BSP_ENV_SENSOR_GetValue(0, ENV_TEMPERATURE, &value);
    ret = SENSOR_OK;
  }
  return ret;
}

static int32_t TemperatureSensor_Disable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_ENV_SENSOR_Disable(0, ENV_TEMPERATURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static uint32_t TemperatureSensor_GetOverflow (void) {
  return 0U;
}

static uint32_t TemperatureSensor_ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;
  float    value;

  (void)num_samples;

  ret = HTS221_TEMP_Get_DRDY_Status(Env_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_ENV_SENSOR_GetValue(0, ENV_TEMPERATURE, &value) == BSP_ERROR_NONE) {
      memcpy(buf, &value, sizeof(float));
      num = 1U;
    }
  }
  return num;
}

sensorDrvHW_t sensorDrvHW_0 = {
  NULL,
  TemperatureSensor_Enable,
  TemperatureSensor_Disable,
  TemperatureSensor_GetOverflow,
  TemperatureSensor_ReadSamples,
  NULL
};


// Humidity Sensor

static int32_t HumiditySensor_Enable (void) {
  int32_t ret = SENSOR_ERROR;
  float   value;

  if (BSP_ENV_SENSOR_Enable(0, ENV_HUMIDITY) == BSP_ERROR_NONE) {
    BSP_ENV_SENSOR_GetValue(0, ENV_HUMIDITY, &value);
    ret = SENSOR_OK;
  }
  return ret;
}

static int32_t HumiditySensor_Disable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_ENV_SENSOR_Disable(0, ENV_HUMIDITY) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static uint32_t HumiditySensor_GetOverflow (void) {
  return 0U;
}

static uint32_t HumiditySensor_ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;
  float    value;

  (void)num_samples;

  ret = HTS221_HUM_Get_DRDY_Status(Env_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_ENV_SENSOR_GetValue(0, ENV_HUMIDITY, &value) == BSP_ERROR_NONE) {
      memcpy(buf, &value, sizeof(float));
      num = 1U;
    }
  }
  return num;
}

sensorDrvHW_t sensorDrvHW_1 = {
  NULL,
  HumiditySensor_Enable,
  HumiditySensor_Disable,
  HumiditySensor_GetOverflow,
  HumiditySensor_ReadSamples,
  NULL
};


// Pressure Sensor

static int32_t PressureSensor_Enable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_ENV_SENSOR_Enable(1, ENV_PRESSURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static int32_t PressureSensor_Disable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_ENV_SENSOR_Disable(1, ENV_PRESSURE) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static uint32_t PressureSensor_GetOverflow (void) {
  return 0U;
}

static uint32_t PressureSensor_ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;
  float    value;

  (void)num_samples;

  ret = LPS22HH_PRESS_Get_DRDY_Status(Env_Sensor_CompObj[1], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_ENV_SENSOR_GetValue(1, ENV_PRESSURE, &value) == BSP_ERROR_NONE) {
      memcpy(buf, &value, sizeof(float));
      num = 1U;
    }
  }
  return num;
}

sensorDrvHW_t sensorDrvHW_2 = {
  NULL,
  PressureSensor_Enable,
  PressureSensor_Disable,
  PressureSensor_GetOverflow,
  PressureSensor_ReadSamples,
  NULL
};


// Accelerometer

static int32_t Accelerometer_Enable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_MOTION_SENSOR_Enable(0, MOTION_ACCELERO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static int32_t Accelerometer_Disable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_MOTION_SENSOR_Disable(0, MOTION_ACCELERO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static uint32_t Accelerometer_GetOverflow (void) {
  return 0U;
}

static uint32_t Accelerometer_ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;
  BSP_MOTION_SENSOR_AxesRaw_t axes;

  (void)num_samples;

  ret = ISM330DHCX_ACC_Get_DRDY_Status(Motion_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_MOTION_SENSOR_GetAxesRaw(0, MOTION_ACCELERO, &axes) == BSP_ERROR_NONE) {
      memcpy(buf, &axes, sizeof(BSP_MOTION_SENSOR_AxesRaw_t));
      num = 1U;
    }
  }
  return num;
}

sensorDrvHW_t sensorDrvHW_3 = {
  NULL,
  Accelerometer_Enable,
  Accelerometer_Disable,
  Accelerometer_GetOverflow,
  Accelerometer_ReadSamples,
  NULL
};


// Gyroscope

static int32_t Gyroscope_Enable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_MOTION_SENSOR_Enable(0, MOTION_GYRO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static int32_t Gyroscope_Disable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_MOTION_SENSOR_Disable(0, MOTION_GYRO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static uint32_t Gyroscope_GetOverflow (void) {
  return 0U;
}

static uint32_t Gyroscope_ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;
  BSP_MOTION_SENSOR_AxesRaw_t axes;

  (void)num_samples;

  ret = ISM330DHCX_GYRO_Get_DRDY_Status(Motion_Sensor_CompObj[0], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_MOTION_SENSOR_GetAxesRaw(0, MOTION_GYRO, &axes) == BSP_ERROR_NONE) {
      memcpy(buf, &axes, sizeof(BSP_MOTION_SENSOR_AxesRaw_t));
      num = 1U;
    }
  }
  return num;
}

sensorDrvHW_t sensorDrvHW_4 = {
  NULL,
  Gyroscope_Enable,
  Gyroscope_Disable,
  Gyroscope_GetOverflow,
  Gyroscope_ReadSamples,
  NULL
};


// Magnetometer

static int32_t Magnetometer_Enable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_MOTION_SENSOR_Enable(1, MOTION_MAGNETO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static int32_t Magnetometer_Disable (void) {
  int32_t ret = SENSOR_ERROR;

  if (BSP_MOTION_SENSOR_Disable(1, MOTION_MAGNETO) == BSP_ERROR_NONE) {
    ret = SENSOR_OK;
  }
  return ret;
}

static uint32_t Magnetometer_GetOverflow (void) {
  return 0U;
}

static uint32_t Magnetometer_ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num = 0U;
  int32_t  ret;
  uint8_t  stat;
  BSP_MOTION_SENSOR_AxesRaw_t axes;

  (void)num_samples;

  ret = IIS2MDC_MAG_Get_DRDY_Status(Motion_Sensor_CompObj[1], &stat);
  if ((ret == 0) && (stat != 0U)) {
    if (BSP_MOTION_SENSOR_GetAxesRaw(1, MOTION_MAGNETO, &axes) == BSP_ERROR_NONE) {
      memcpy(buf, &axes, sizeof(BSP_MOTION_SENSOR_AxesRaw_t));
      num = 1U;
    }
  }
  return num;
}

sensorDrvHW_t sensorDrvHW_5 = {
  NULL,
  Magnetometer_Enable,
  Magnetometer_Disable,
  Magnetometer_GetOverflow,
  Magnetometer_ReadSamples,
  NULL
};
