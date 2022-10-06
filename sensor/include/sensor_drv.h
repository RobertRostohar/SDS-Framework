/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SENSOR_DRV_H
#define SENSOR_DRV_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// ==== Sensor Driver Interface ====

/// Control block
typedef struct sensor_t;

/// Configuration
typedef const strcut {
  char    *name;                            ///< Sensor name
  uint32_t sample_size;                     ///< Sample size in bytes
  uint32_t sample_interval;                 ///< Sample interval in microseconds
  uint32_t data_threshold;                  ///< Data event threshold in bytes
  void    *block_mem;                       ///< Block memory start address
  uint32_t block_size;                      ///< Block size in bytes
  uint32_t block_num;                       ///< Number of blocks
} sensorConfig_t;

/// Status
typedef struct {
  uint32_t active   :  1;                   ///< Active flag: 1=active(enabled), 0=inactive(disabled)
  uint32_t overflow :  1;                   ///< Oveflow flag (cleared on read)
  uint32_t reserved : 30;
} sensorStatus_t;

/// Function return codes
#define SENSOR_OK               (0)         ///< Operation completed successfully
#define SENSOR_ERROR            (-1)        ///< Operation failed

/// Events
#define SENSOR_EVENT_DATA       (1UL << 0)  ///< Data available
#define SENSOR_EVENT_OVERFLOW   (1UL << 1)  ///< Overflow detected

/// Event callback function
typedef void (*sensorEvent_t) (sensor_t *handle, uint32_t event);

/**
  \fn          sensor_t *sensorGetHandle (const char *name)
  \brief       Get sensor handle.
  \param[in]   name        sensor name (pointer to NULL terminated string)
  \return      pointer to sensor_t
*/
sensor_t *sensorGetHandle (const char *name);

/**
  \fn          sensorConfig_t *sensorGetConfig (sensor_t *handle)
  \brief       Get sensor configuration.
  \param[in]   handle      pointer to sensor_t
  \return      pointer to \ref sensorConfig_t
*/
sensorConfig_t *sensorGetConfig (sensor_t *handle);

/**
  \fn          int32_t sensorRegisterEvents (sensor_t *handle, sensorEvent_t event_cb, uint32_t event_mask)
  \brief       Register sensor events.
  \param[in]   handle      pointer to sensor_t
  \param[in]   event_cb    pointer to \ref sensorEvent_t
  \param[in]   event_mask  event mask
  \return      return code
*/
int32_t sensorRegisterEvents (sensor_t *handle, sensorEvent_t event_cb, uint32_t event_mask);

/**
  \fn          int32_t sensorEnable (sensor_t *handle)
  \brief       Enable sensor.
  \param[in]   handle      pointer to sensor_t
  \return      return code
*/
int32_t sensorEnable (sensor_t *handle);

/**
  \fn          int32_t sensorDisable (sensor_t *handle)
  \brief       Disable sensor.
  \param[in]   handle      pointer to sensor_t
  \return      return code
*/
int32_t sensorDisable (sensor_t *handle);

/**
  \fn          uint32_t sensorReadSamples (sensor_t *handle, uint32_t num_samples, void *buf, uint32_t buf_size)
  \brief       Read samples from sensor.
  \param[in]   handle      pointer to sensor_t
  \param[in]   num_samples maximum number of samples to read
  \param[out]  buf         pointer to buffer for samples
  \param[in]   buf_size    buffer size in bytes
  \return      number of samples read
*/
uint32_t sensorReadSamples (sensor_t *handle, uint32_t num_samples, void *buf, uint32_t buf_size);

/**
  \fn          sensorStatus_t sensorGetStatus (sensor_t *handle)
  \brief       Get sensor status.
  \param[in]   handle      pointer to sensor_t
  \return      \ref sensorStatus_t
*/
sensorStatus_t sensorGetStatus (sensor_t *handle);

/**
  \fn          void *sensorGetBlockData (sensor_t *handle)
  \brief       Get block data.
  \param[in]   handle      pointer to sensor_t
  \return      pointer to block data
*/
void *sensorGetBlockData (sensor_t *handle);

#ifdef  __cplusplus
}
#endif

#endif  /* SENSOR_DRV_H */
