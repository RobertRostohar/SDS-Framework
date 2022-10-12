/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SDS_PLAY_H
#define SDS_PLAY_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// ==== SDS Player ====

/// Identifier
typedef void *sdsPlayId_t;

/// Function return codes
#define SDS_PLAY_OK             (0)         ///< Operation completed successfully
#define SDS_PLAY_ERROR          (-1)        ///< Operation failed

/**
  \fn          sdsPlayId_t sdsPlayOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size)
  \brief       Open player stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   buf            pointer to buffer for stream
  \param[in]   buf_size       buffer size in bytes
  \param[in]   record_size    record size in bytes
  \return      \ref sdsPlayId_t
*/
sdsPlayId_t sdsPlayOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size);

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

#endif  /* SDS_PLAY_H */
