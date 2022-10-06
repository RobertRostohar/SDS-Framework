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

/// Control block
typedef struct sds_t;

/// Events
#define SDS_EVENT_DATA_LOW      (1UL << 0)  ///< Data bellow threshold
#define SDS_EVENT_DATA_HIGH     (1UL << 1)  ///< Data above or equal to threshold

/// Event callback function
typedef void (*sdsEvent_t) (sds_t *handle, uint32_t event, void *arg);

/**
  \fn          sds_t *sdsOpen (void *buf, uint32_t buf_size, uint32_t record_size)
  \brief       Open stream.
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   threshold_low  data low threshold in bytes
  \param[in]   threshold_high data high threshold in bytes
  \return      pointer to sds_t
*/
sds_t *sdsOpen (void *buf, uint32_t buf_size, uint32_t threshold_low, uint32_t threshold_high);

/**
  \fn          int32_t sdsClose (sds_t *handle)
  \brief       Close stream.
  \param[in]   handle      pointer to sds_t
  \return      return code
*/
int32_t sdsClose (sds_t *handle);

/**
  \fn          int32_t  sdsRegisterEvents (sds_t *handle, sdsEvent_t event_cb, uint32_t event_mask, void *arg)
  \brief       Register stream events.
  \param[in]   handle         pointer to sds_t
  \param[in]   event_cb       pointer to \ref sdsEvent_t
  \param[in]   event_mask     event mask
  \param[in]   arg            user argument
  \return      return code
*/
int32_t sdsRegisterEvents (sds_t *handle, sdsEvent_t event_cb, uint32_t event_mask, void *arg);

/**
  \fn          uint32_t sdsWrite (sds_t *handle, void *buf, uint32_t buf_size)
  \brief       Write data to stream.
  \param[in]   handle         pointer to sds_t
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes written
*/
uint32_t sdsWrite (sds_t *handle, void *buf, uint32_t buf_size);

/**
  \fn          uint32_t sdsRead (sds_t *handle, void *buf, uint32_t buf_size)
  \brief       Read data from stream.
  \param[in]   handle         pointer to sds_t
  \param[out]  buf            pointer to buffer for data to read
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes read
*/
uint32_t sdsRead (sds_t *handle, void *buf, uint32_t buf_size);

/**
  \fn          uint32_t sdsGetCount (sds_t *handle)
  \brief       Get data count in stream.
  \param[in]   handle         pointer to sds_t
  \return      number of bytes in stream
*/
uint32_t sdsGetCount (sds_t *handle);


// ==== SDS Recorder ====

/// Control block
typedef struct sdsRec_t;

/**
  \fn          int32_t *sdsRecInit (void)
  \brief       Initialize recorder.
  \return      return code
*/
int32_t *sdsRecInit (void);

/**
  \fn          int32_t *sdsRecUninit (void)
  \brief       Uninitialize recorder.
  \return      return code
*/
int32_t *sdsRecUninit (void);

/**
  \fn          sdsRec_t *sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size)
  \brief       Open recorder stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   data_threshold data high threshold size in bytes
  \return      pointer to sdsRec_t
*/
sdsRec_t *sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold);

/**
  \fn          int32_t sdsRecClose (sdsRec_t *handle)
  \brief       Close recorder stream.
  \param[in]   handle         pointer to sdsRec_t
  \return      return code
*/
int32_t sdsRecClose (sdsRec_t *handle);

/**
  \fn          sdsRecWrite (sdsRec_t *handle, void *buf, uint32_t buf_size)
  \brief       Write data to recorder stream.
  \param[in]   handle         pointer to sdsRec_t
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes written
*/
uint32_t sdsRecWrite (sdsRec_t *handle, void *buf, uint32_t buf_size);


// ==== SDS Player ====

/// Control block
typedef struct sdsPlay_t;

/**
  \fn          int32_t *sdsPlayInit (void)
  \brief       Initialize player.
  \return      return code
*/
int32_t *sdsPlayInit (void);

/**
  \fn          int32_t *sdsPlayUninit (void)
  \brief       Uninitialize player.
  \return      return code
*/
int32_t *sdsPlayUninit (void);

/**
  \fn          sdsPlay_t *sdsPlayOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size)
  \brief       Open player stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   data_threshold data low threshold size in bytes
  \return      pointer to sdsPlay_t
*/
sdsPlay_t *sdsPlayOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold);

/**
  \fn          int32_t sdsPlayClose (sdsPlay_t *handle)
  \brief       Close player stream.
  \param[in]   handle         pointer to sdsPlay_t
  \return      return code
*/
int32_t sdsPlayClose (sdsPlay_t *handle);

/**
  \fn          sdsPlayRead (sdsPlay_t *handle, void *buf, uint32_t buf_size)
  \brief       Write data to Player stream.
  \param[in]   handle         pointer to sdsPlay_t
  \param[out]  buf            pointer to buffer for data to read
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes read
*/
uint32_t sdsPlayRead (sdsPlay_t *handle, void *buf, uint32_t buf_size);


#ifdef  __cplusplus
}
#endif

#endif  /* SDS_H */

