/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SDSIO_H
#define SDSIO_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// ==== Synchronous Data Stream Input/Output (SDS I/O) ====

/// Identifier
typedef void *sdsioId_t;

/// Open Mode
typdef enum {
  sdsioRead  = 0,               ///< Open for read (binary)
  sdsioWrite = 1                ///< Open for write (binary)
} sdsioMode_t;

/// Function return codes
#define SDSIO_OK                (0)         ///< Operation completed successfully
#define SDSIO_ERROR             (-1)        ///< Operation failed

/**
  \fn          sdsioId_t sdsioOpen (const char *name, sdsioMode_t mode)
  \brief       Open I/O stream.
  \param[in]   name           stream name (pointer to NULL terminated string)
  \param[in]   mode           \ref sdsioMode_t
  \return      \ref sdsioId_t
*/
sdsioId_t sdsioOpen (const char *name, sdsioMode_t mode);

/**
  \fn          int32_t sdsioClose (sdsioId_t id)
  \brief       Close I/O stream.
  \param[in]   id             \ref sdsioId_t
  \return      return code
*/
int32_t sdsioClose (sdsioId_t id);

/**
  \fn          uint32_t sdsioWrite (sdsioId_t id, const void *buf, uint32_t buf_size)
  \brief       Write data to I/O stream.
  \param[in]   id             \ref sdsioId_t
  \param[in]   buf            pointer to buffer with data to write
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes written
*/
uint32_t sdsioWrite (sdsioId_t id, const void *buf, uint32_t buf_size);

/**
  \fn          uint32_t sdsioRead (sdsioId_t id, void *buf, uint32_t buf_size)
  \brief       Read data from I/O stream.
  \param[in]   id             \ref sdsioId_t
  \param[out]  buf            pointer to buffer for data to read
  \param[in]   buf_size       buffer size in bytes
  \return      number of bytes read
*/
uint32_t sdsioRead (sdsioId_t id, void *buf, uint32_t buf_size);

#ifdef  __cplusplus
}
#endif

#endif  /* SDSIO_H */
