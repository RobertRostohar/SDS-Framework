/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SDS Recorder
 */
#include <string.h>

#include "cmsis_os2.h"
#include "sds.h"
#include "sdsRec_if.h"

/// Configuration
#define SDS_REC_MAX_STREAMS             4UL


/// Control block
typedef struct {
  uint32_t      idx;
  const char   *name;
  sdsId_t      *stream;
  uint32_t      threshold;
  sdsRecIfId_t  interface;
} sdsRec_t;

#if SDS_REC_MAX_STREAMS > 30
#error "Maximmum number of SDS Recorder streams is 30!"
#endif

#define SDS_REC_THREAD_TERMINATE_FLAG   (1UL << 30)
#define SDS_REC_EVENT_FLAG_MASK         ((1UL << SDS_REC_MAX_STREAMS) - 1) | SDS_REC_THREAD_TERMINATE_FLAG

osThreadId_t sdsRecThread_id = 0;

/// Allocate SDS Recorder control blocks
sdsRec_t rec_a[SDS_REC_MAX_STREAMS] = {0};

/// Helper functions

static sdsRec_t * sdsRecGetStream (void) {
  uint32_t i;
  sdsRec_t *rec = NULL;

  for (i = 0U; i < SDS_REC_MAX_STREAMS; i++) {
    if (rec_a[i].name == NULL) {
      rec = &rec_a[i];
      rec->idx = i;
      break;
    }
  }
  return rec;
}

static void sdsRecReleaseStream (sdsRec_t * rec) {
  memset(rec, 0, sizeof(sdsRec_t));
}

/**
  Event callback
*/
static void sdsRecEventCallback (sdsId_t id, uint32_t event, void *arg) {
  uint32_t cnt;
  sdsRec_t *rec = arg;

  cnt = sdsGetCount(rec->stream);
  if (cnt >= rec->threshold) {
    osThreadFlagsSet(sdsRecThread_id, (1U << rec->idx));
  }
}

/**
  Recorder thread
*/
static void sdsRecThread (void *arg) {
  uint32_t  i;
  uint32_t  flags = 0U;
  sdsRec_t *rec;

  while (1) {
    flags = osThreadFlagsWait(SDS_REC_EVENT_FLAG_MASK, osFlagsWaitAny, osWaitForever);
    if ((flags & osFlagsError) == 0U) {
      if ((flags & SDS_REC_THREAD_TERMINATE_FLAG) != 0U) {
        // exit thread
        break;
      } else {
        for (i = 0U; i < SDS_REC_MAX_STREAMS; i++) {
          if (flags & (1U << i)) {
            rec = &rec_a[i];
            if (sdsGetCount(rec->stream) >= rec->threshold) {
              sdsRecIfDataAvailable(rec->interface);
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
  int32_t ret;

  // Initilaize interface
  ret = sdsRecIfInit();

  if (ret == SDS_OK) {
    sdsRecThread_id = osThreadNew(sdsRecThread, NULL, NULL);
    if (sdsRecThread_id != NULL)  {
      ret = SDS_OK;
    }
  }
  return ret;
}

/**
  Uninitialize recorder.
*/
int32_t sdsRecUninit (void) {
  osThreadFlagsSet(sdsRecThread_id, SDS_REC_THREAD_TERMINATE_FLAG);
  sdsRecIfUninit();
  return SDS_OK;
}

/**
  Open recorder stream
*/
sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t data_threshold) {
  sdsRec_t *rec;

  rec = sdsRecGetStream();
  if (rec != NULL) {

    rec->interface = sdsRecIfOpen(name, data_threshold, rec);

    if (rec->interface != NULL) {
      rec->stream = sdsOpen(buf, buf_size, 0U, data_threshold);
      if (rec->stream != NULL) {
        sdsRegisterEvents(rec->stream, sdsRecEventCallback, SDS_EVENT_DATA_HIGH, rec);
      } else {
        sdsRecIfClose(rec->interface);
        sdsRecReleaseStream(rec);
        rec = NULL;
      }
    }
  }
  return rec;
}

/**
  Close recorder stream.
*/
int32_t sdsRecClose (sdsRecId_t id) {
  int32_t   ret = SDS_ERROR;
  sdsRec_t *rec = id;

  if (rec != NULL) {
    sdsClose(rec->stream);
    sdsRecIfClose(rec->interface);
    sdsRecReleaseStream(rec);
    ret = SDS_OK;
  }

  return ret;
}

/**
  Write data to recorder stream.
*/
uint32_t sdsRecWrite (sdsRecId_t id, void *buf, uint32_t buf_size) {
  uint32_t  num = 0U;
  sdsRec_t *rec = id;

  if (rec != NULL) {
    num = sdsWrite(rec->stream, buf, buf_size);
  }
  return num;
}

/**
  Read data from recorder stream.
*/
uint32_t sdsRecRead (sdsRecId_t id, void *buf, uint32_t buf_size) {
  uint32_t  num = 0U;
  sdsRec_t *rec = id;

  if (rec != NULL) {
    num = sdsRead(rec->stream, buf, buf_size);
  }
  return num;
}
