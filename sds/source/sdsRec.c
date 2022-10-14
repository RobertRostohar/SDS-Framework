/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SDS Recorder
 */
#include <string.h>

#include "cmsis_os2.h"
#include "sds.h"
#include "sdsio.h"
#include "sds_rec.h"

/// Configuration
#ifndef SDS_REC_MAX_STREAMS
#define SDS_REC_MAX_STREAMS         8U
#endif
#ifndef SDS_REC_MAX_RECORD_SIZE
#define SDS_REC_MAX_RECORD_SIZE     1024U
#endif
#ifdef  SDSIO_NO_BUFFER
#ifndef SDSIO_HEADER_SIZE
#define SDSIO_HEADER_SIZE           32U
#endif
#ifndef SDSIO_TAIL_SIZE
#define SDSIO_TAIL_SIZE             0U
#endif
#endif

/// Control block
typedef struct {
  uint32_t      idx;
  const char   *name;
  sdsId_t      *stream;
  uint32_t      record_size;
  sdsioId_t     sdsio;
} sdsRec_t;

#if SDS_REC_MAX_STREAMS >= 30
#error "Maximmum number of SDS Recorder streams is 29!"
#endif

#define SDS_REC_THREAD_TERMINATE_FLAG   (1UL << 30)
#define SDS_REC_EVENT_FLAG_MASK         ((1UL << SDS_REC_MAX_STREAMS) - 1) | SDS_REC_THREAD_TERMINATE_FLAG

static osThreadId_t sdsRecThread_id = 0;

#ifndef  SDSIO_NO_BUFFER
static uint8_t    rec_buf[SDS_REC_MAX_RECORD_SIZE];
static uint8_t *p_rec_buf = rec_buf;
#else
static uint8_t    rec_buf[SDS_REC_MAX_RECORD_SIZE + SDSIO_HEADER_SIZE + SDSIO_TAIL_SIZE];
static uint8_t *p_rec_buf = &rec_buf[SDSIO_HEADER_SIZE];
#endif

/// SDS Recorder control blocks
static sdsRec_t    rec_cb[SDS_REC_MAX_STREAMS];
static sdsRec_t *p_rec_cb[SDS_REC_MAX_STREAMS] = {0};

/// Helper functions

static sdsRec_t * sdsRecAlloc (void) {
  sdsRec_t *rec = NULL;
  uint32_t i;

  for (i = 0U; i < SDS_REC_MAX_STREAMS; i++) {
    if (p_rec_cb[i] == NULL) {
      p_rec_cb[i] = &rec_cb[i];
      rec = p_rec_cb[i];
      rec->idx = i;
      break;
    }
  }
  return rec;
}

static void sdsRecFree (sdsRec_t * rec) {
  uint32_t i;

  if (rec != NULL) {
    for (i = 0U; i < SDS_REC_MAX_STREAMS; i++) {
      if (p_rec_cb[i] == rec) {
        p_rec_cb[i] = NULL;
        break;
      }
    }
  }
  memset(rec, 0, sizeof(sdsRec_t));
}

/**
  Event callback
*/
static void sdsRecEventCallback (sdsId_t id, uint32_t event, void *arg) {
  sdsRec_t *rec = arg;

  (void)id;
  (void)event;      // Only SDS_EVENT_DATA_HIGH is masked, no need to check which event is set

  osThreadFlagsSet(sdsRecThread_id, (1U << rec->idx));
}

/**
  Recorder thread
*/
static void sdsRecThread (void *arg) {
  sdsRec_t *rec;
  uint32_t  i, cnt, flags = 0U;

  (void)arg;

  while (1) {
    flags = osThreadFlagsWait(SDS_REC_EVENT_FLAG_MASK, osFlagsWaitAny, osWaitForever);
    if ((flags & osFlagsError) == 0U) {
      if ((flags & SDS_REC_THREAD_TERMINATE_FLAG) != 0U) {
        // exit thread
        break;
      } else {
        for (i = 0U; i < SDS_REC_MAX_STREAMS; i++) {
          if (flags & (1U << i)) {
            rec = &rec_cb[i];
            while (sdsGetCount(rec->stream) >= rec->record_size) {
              cnt = sdsRead(rec->stream, p_rec_buf, rec->record_size);
              sdsioWrite(rec->sdsio, p_rec_buf, cnt);
            }
          }
        }
      }
    }
  }
}

/// SDS Recorder functions

/**
  Initialize recorder
*/
int32_t sdsRecInit (void) {
  int32_t ret = SDS_REC_ERROR;

  sdsRecThread_id = osThreadNew(sdsRecThread, NULL, NULL);
  if (sdsRecThread_id != NULL)  {
    ret = SDS_OK;
  }
  return ret;
}

/**
  Uninitialize recorder.
*/
int32_t sdsRecUninit (void) {
  osThreadFlagsSet(sdsRecThread_id, SDS_REC_THREAD_TERMINATE_FLAG);
  return SDS_OK;
}

/**
  Open recorder stream
*/
sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size) {
  sdsRec_t *rec = NULL;

  if ((name != NULL) && (buf != NULL) && (buf_size != 0U) && (record_size != 0U)) {
    rec = sdsRecAlloc();
    if (rec != NULL) {
      rec->sdsio = sdsioOpen(name, sdsioModeWrite); 
      if (rec->sdsio != NULL) {
        rec->stream = sdsOpen(buf, buf_size, 0U, record_size);
        if (rec->stream != NULL) {
          sdsRegisterEvents(rec->stream, sdsRecEventCallback, SDS_EVENT_DATA_HIGH, rec);
          rec->record_size = record_size;
        } else {
          sdsioClose(rec->sdsio);
          sdsRecFree(rec);
          rec = NULL;
        }
      }
    }
  }
  return rec;
}

/**
  Close recorder stream.
*/
int32_t sdsRecClose (sdsRecId_t id) {
  sdsRec_t *rec = id;
  int32_t   ret = SDS_ERROR;

  if (rec != NULL) {
    sdsClose(rec->stream);
    sdsioClose(rec->sdsio);
    sdsRecFree(rec);
    ret = SDS_OK;
  }
  return ret;
}

/**
  Write data to recorder stream.
*/
uint32_t sdsRecWrite (sdsRecId_t id, const void *buf, uint32_t buf_size) {
  sdsRec_t *rec = id;
  uint32_t  num = 0U;

  if ((rec != NULL) && (buf != NULL) && (buf_size != 0U)) {
    num = sdsWrite(rec->stream, buf, buf_size);
  }
  return num;
}
