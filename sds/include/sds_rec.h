/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SDS_REC_H
#define SDS_REC_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// ==== SDS Recorder ====

/// Identifier
typedef void *sdsRecId_t;

/// Function return codes
#define SDS_REC_OK              (0)         ///< Operation completed successfully
#define SDS_REC_ERROR           (-1)        ///< Operation failed

/**
  \fn          sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size)
  \brief       Open recorder stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   record_size    record size in bytes
  \return      \ref sdsRecId_t
*/
sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size);

/**
  \fn          int32_t sdsRecClose (sdsRecId_t id)
  \brief       Close recorder stream.
  \param[in]   id             \ref sdsRecId_t
  \return      return code
*/
int32_t sdsRecClose (sdsRecId_t id);

/**
  \fn          uint32_t sdsRecWrite (sdsRecId_t id, const void *buf, uint32_t buf_size)
  \brief       Write data to recorder stream.
  \param[in]   id             \ref sdsRecId_t
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes written
*/
uint32_t sdsRecWrite (sdsRecId_t id, const void *buf, uint32_t buf_size);

#ifdef  __cplusplus
}
#endif

#endif  /* SDS_REC_H */
