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

/// Identifier
typedef void *sensorId_t;

/// Configuration
typedef const struct {
  char    *name;                            ///< Sensor name
  uint32_t sample_size;                     ///< Sample size in bytes
  uint32_t sample_interval;                 ///< Sample interval in microseconds
  uint32_t data_threshold;                  ///< Data event threshold in number of samples/blocks
  uint32_t fifo_size;                       ///< Sample FIFO size in bytes
  uint32_t block_size;                      ///< Block size in bytes
  uint32_t block_num;                       ///< Number of blocks
} sensorConfig_t;

/// Status
typedef struct {
  uint32_t active   :  1;                   ///< Active flag: 1=active(enabled), 0=inactive(disabled)
  uint32_t overflow :  1;                   ///< Overflow flag (cleared on read)
  uint32_t reserved : 30;
} sensorStatus_t;

/// Functions (back-end)
typedef const struct {
  int32_t  (*Enable)      (void);
  int32_t  (*Disable)     (void);
  uint32_t (*ReadSamples) (uint32_t num_samples, void *buf);
  uint32_t (*GetOverflow) (void);
  void *   (*GetBlockData)(void);
} sensorFunc_t;

/// Function return codes
#define SENSOR_OK               (0)         ///< Operation completed successfully
#define SENSOR_ERROR            (-1)        ///< Operation failed

/// Events
#define SENSOR_EVENT_DATA       (1UL << 0)  ///< Data available
#define SENSOR_EVENT_OVERFLOW   (1UL << 1)  ///< Overflow detected

/// Event callback function
typedef void (*sensorEvent_t) (sensorId_t id, uint32_t event);

/**
  \fn          sensorId_t sensorGetId (const char *name)
  \brief       Get sensor identifier.
  \param[in]   name        sensor name (pointer to NULL terminated string)
  \return      \ref sensorId_t
*/
sensorId_t sensorGetId (const char *name);

/**
  \fn          sensorConfig_t *sensorGetConfig (sensorId_t id)
  \brief       Get sensor configuration.
  \param[in]   id          \ref sensorId_t
  \return      pointer to \ref sensorConfig_t
*/
sensorConfig_t *sensorGetConfig (sensorId_t id);

/**
  \fn          int32_t sensorRegisterEvents (sensorId_t id, sensorEvent_t event_cb, uint32_t event_mask)
  \brief       Register sensor events.
  \param[in]   id          \ref sensorId_t
  \param[in]   event_cb    pointer to \ref sensorEvent_t
  \param[in]   event_mask  event mask
  \return      return code
*/
int32_t sensorRegisterEvents (sensorId_t id, sensorEvent_t event_cb, uint32_t event_mask);

/**
  \fn          int32_t sensorEnable (sensorId_t id)
  \brief       Enable sensor.
  \param[in]   id          \ref sensorId_t
  \return      return code
*/
int32_t sensorEnable (sensorId_t id);

/**
  \fn          int32_t sensorDisable (sensorId_t id)
  \brief       Disable sensor.
  \param[in]   id          \ref sensorId_t
  \return      return code
*/
int32_t sensorDisable (sensorId_t id);

/**
  \fn          uint32_t sensorReadSamples (sensorId_t id, uint32_t num_samples, void *buf, uint32_t buf_size)
  \brief       Read samples from sensor.
  \param[in]   id          \ref sensorId_t
  \param[in]   num_samples maximum number of samples to read
  \param[out]  buf         pointer to buffer for samples
  \param[in]   buf_size    buffer size in bytes
  \return      number of samples read
*/
uint32_t sensorReadSamples (sensorId_t id, uint32_t num_samples, void *buf, uint32_t buf_size);

/**
  \fn          sensorStatus_t sensorGetStatus (sensorId_t id)
  \brief       Get sensor status.
  \param[in]   id          \ref sensorId_t
  \return      \ref sensorStatus_t
*/
sensorStatus_t sensorGetStatus (sensorId_t id);

/**
  \fn          void *sensorGetBlockData (sensorId_t id)
  \brief       Get block data.
  \param[in]   id          \ref sensorId_t
  \return      pointer to block data
*/
void *sensorGetBlockData (sensorId_t id);

#ifdef  __cplusplus
}
#endif

#endif  /* SENSOR_DRV_H */
