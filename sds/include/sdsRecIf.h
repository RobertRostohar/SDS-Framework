/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SDS_REC_IF_H
#define SDS_REC_IF_H

#ifdef  __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/// Control block
typedef void *sdsRecIfId_t;

/// Function return codes
#define SDS_REC_IF_OK            (0)         ///< Operation completed successfully
#define SDS_REC_IF_ERROR         (-1)        ///< Operation failed

/**
  \fn          int32_t sdsRecIfInit (void)
  \brief       Initialize recorder interface.
  \return      return code
*/
int32_t sdsRecIfInit (void);

/**
  \fn          int32_t sdsRecIfUninit (void)
  \brief       Uninitialize recorder interface.
  \return      return code
*/
int32_t sdsRecIfUninit (void);

/**
  \fn          sdsRecIfId_t sdsRecIfOpen (const char *name, uint32_t record_size)
  \brief       Open recorder interface.
  \param[in]   name           name (pointer to NULL terminated string)
  \param[in]   record_size    data size of a record packet
  \param[in]   arg            user argument
  \return      \ref sdsRecIfId_t
*/
sdsRecIfId_t sdsRecIfOpen (const char *name, uint32_t record_size, void *arg);

/**
  \fn          int32_t sdsRecIfClose (sdsRecIfId_t id)
  \brief       Close recorder interface.
  \param[in]   id             \ref sdsRecIfId_t
  \return      return code
*/
int32_t sdsRecIfClose (sdsRecIfId_t id);

/**
  \fn          int32_t sdsRecIfDataAvailable (sdsRecIfId_t id)
  \brief       Notify recorder interface that data is available for transfer.
  \param[in]   id             \ref sdsRecIfId_t
  \return      return code
*/
int32_t sdsRecIfDataAvailable (sdsRecIfId_t id);


#ifdef  __cplusplus
}
#endif

#endif  /* SDS_REC_IF_H */
