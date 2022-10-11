/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SDS_H
#define SDS_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>


// ==== Synchronous Data Stream ====

/// Identifier
typedef void *sdsId_t;

/// Function return codes
#define SDS_OK                  (0)         ///< Operation completed successfully
#define SDS_ERROR               (-1)        ///< Operation failed

/// Events
#define SDS_EVENT_DATA_LOW      (1UL << 0)  ///< Data bellow threshold
#define SDS_EVENT_DATA_HIGH     (1UL << 1)  ///< Data above or equal to threshold

/// Event callback function
typedef void (*sdsEvent_t) (sdsId_t id, uint32_t event, void *arg);

/**
  \fn          sdsId_t sdsOpen (void *buf, uint32_t buf_size, uint32_t record_size)
  \brief       Open stream.
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   threshold_low  data low threshold in bytes
  \param[in]   threshold_high data high threshold in bytes
  \return      \ref sdsId_t
*/
sdsId_t sdsOpen (void *buf, uint32_t buf_size, uint32_t threshold_low, uint32_t threshold_high);

/**
  \fn          int32_t sdsClose (sdsId_t id)
  \brief       Close stream.
  \param[in]   id             \ref sdsId_t
  \return      return code
*/
int32_t sdsClose (sdsId_t id);

/**
  \fn          int32_t  sdsRegisterEvents (sdsId_t id, sdsEvent_t event_cb, uint32_t event_mask, void *arg)
  \brief       Register stream events.
  \param[in]   id             \ref sdsId_t
  \param[in]   event_cb       pointer to \ref sdsEvent_t
  \param[in]   event_mask     event mask
  \param[in]   arg            user argument
  \return      return code
*/
int32_t sdsRegisterEvents (sdsId_t id, sdsEvent_t event_cb, uint32_t event_mask, void *arg);

/**
  \fn          uint32_t sdsWrite (sdsId_t id, void *buf, uint32_t buf_size)
  \brief       Write data to stream.
  \param[in]   id             \ref sdsId_t
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes written
*/
uint32_t sdsWrite (sdsId_t id, void *buf, uint32_t buf_size);

/**
  \fn          uint32_t sdsRead (sdsId_t id, void *buf, uint32_t buf_size)
  \brief       Read data from stream.
  \param[in]   id             \ref sdsId_t
  \param[out]  buf            pointer to buffer for data to read
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes read
*/
uint32_t sdsRead (sdsId_t id, void *buf, uint32_t buf_size);

/**
  \fn          uint32_t sdsGetCount (sdsId_t id)
  \brief       Get data count in stream.
  \param[in]   id             \ref sdsId_t
  \return      number of bytes in stream
*/
uint32_t sdsGetCount (sdsId_t id);


// ==== SDS Recorder ====

/// Identifier
typedef void *sdsRecId_t;

/**
  \fn          int32_t sdsRecInit (void)
  \brief       Initialize recorder.
  \return      return code
*/
int32_t sdsRecInit (void);

/**
  \fn          int32_t sdsRecUninit (void)
  \brief       Uninitialize recorder.
  \return      return code
*/
int32_t sdsRecUninit (void);

/**
  \fn          sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold)
  \brief       Open recorder stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   data_threshold data high threshold size in bytes
  \return      \ref sdsRecId_t
*/
sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold);

/**
  \fn          int32_t sdsRecClose (sdsRecId_t id)
  \brief       Close recorder stream.
  \param[in]   id             \ref sdsRecId_t
  \return      return code
*/
int32_t sdsRecClose (sdsRecId_t id);

/**
  \fn          uint32_t sdsRecWrite (sdsRecId_t id, void *buf, uint32_t buf_size)
  \brief       Write data to recorder stream.
  \param[in]   id             \ref sdsRecId_t
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes written
*/
uint32_t sdsRecWrite (sdsRecId_t id, void *buf, uint32_t buf_size);


// ==== SDS Player ====

/// Identifier
typedef void *sdsPlayId_t;

/**
  \fn          int32_t sdsPlayInit (void)
  \brief       Initialize player.
  \return      return code
*/
int32_t sdsPlayInit (void);

/**
  \fn          int32_t sdsPlayUninit (void)
  \brief       Uninitialize player.
  \return      return code
*/
int32_t sdsPlayUninit (void);

/**
  \fn          sdsPlayId_t sdsPlayOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold)
  \brief       Open player stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   data_threshold data low threshold size in bytes
  \return      \ref sdsPlayId_t
*/
sdsPlayId_t sdsPlayOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold);

/**
  \fn          int32_t sdsPlayClose (sdsPlayId_t id)
  \brief       Close player stream.
  \param[in]   id             \ref sdsPlayId_t
  \return      return code
*/
int32_t sdsPlayClose (sdsPlayId_t id);

/**
  \fn          uint32_t sdsPlayRead (sdsPlayId_t id, void *buf, uint32_t buf_size)
  \brief       Write data to Player stream.
  \param[in]   id             \ref sdsPlayId_t
  \param[out]  buf            pointer to buffer for data to read
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes read
*/
uint32_t sdsPlayRead (sdsPlayId_t id, void *buf, uint32_t buf_size);


#ifdef  __cplusplus
}
#endif

#endif  /* SDS_H */
