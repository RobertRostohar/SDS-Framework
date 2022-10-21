/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SENSOR_DRV_HW_H
#define SENSOR_DRV_HW_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "sensor_drv.h"

// ==== Sensor Driver Hardware Interface ====

// Function documentation
/**
  \fn          int32_t sensorDrvHW_Enable (void)
  \brief       Enable sensor.
  \return      return code
*/
/**
  \fn          int32_t sensorDrvHW_Disable (void)
  \brief       Disable sensor.
  \return      return code
*/
/**
  \fn          uint32_t sensorDrvHW_ReadSamples (uint32_t num_samples, void *buf)
  \brief       Read samples from sensor.
  \param[in]   num_samples maximum number of samples to read
  \param[out]  buf         pointer to buffer for samples
  \return      number of samples read
*/
/**
  \fn          uint32_t sensorDrvHW_GetOverflow (void)
  \brief       Get overflow status.
  \return      0 = no overflow, 1 = overflow (automatically cleared)
*/
/**
  \fn          void *sensorDrvHW_GetBlockData (void)
  \brief       Get block data.
  \return      pointer to block data
*/

/// Functions
typedef const struct {
  int32_t  (*Enable)      (void);
  int32_t  (*Disable)     (void);
  uint32_t (*ReadSamples) (uint32_t num_samples, void *buf);
  uint32_t (*GetOverflow) (void);
  void *   (*GetBlockData)(void);
} sensorDrvHW_t;

extern sensorDrvHW_t SensorDrvHW_0;
extern sensorDrvHW_t SensorDrvHW_1;
extern sensorDrvHW_t SensorDrvHW_2;
extern sensorDrvHW_t SensorDrvHW_3;
extern sensorDrvHW_t SensorDrvHW_4;
extern sensorDrvHW_t SensorDrvHW_5;
extern sensorDrvHW_t SensorDrvHW_6;
extern sensorDrvHW_t SensorDrvHW_7;

#ifdef  __cplusplus
}
#endif

#endif  /* SENSOR_DRV_HW_H */
