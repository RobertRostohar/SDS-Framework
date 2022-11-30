/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// SDS Recorder

#include <string.h>

#include "sds.h"
#include "sdsio.h"
#include "sds_rec.h"
#include "cmsis_os2.h"

// Configuration
#ifndef SDS_REC_MAX_STREAMS
#define SDS_REC_MAX_STREAMS     8U
#endif
#ifndef SDS_REC_MAX_RECORD_SIZE
#define SDS_REC_MAX_RECORD_SIZE 1024U
#endif
#ifdef  SDSIO_NO_BUFFER
#ifndef SDSIO_HEADER_SIZE
#define SDSIO_HEADER_SIZE       32U
#endif
#ifndef SDSIO_TAIL_SIZE
#define SDSIO_TAIL_SIZE         0U
#endif
#endif

#if SDS_REC_MAX_STREAMS > 31
#error "Maximmum number of SDS Recorder streams is 31!"
#endif

// Control block
typedef struct {
  uint32_t    record_size;
  sdsId_t     stream;
  sdsioId_t   sdsio;
} sdsRec_t;

static sdsRec_t   RecStreams[SDS_REC_MAX_STREAMS] = {0};
static sdsRec_t *pRecStreams[SDS_REC_MAX_STREAMS] = {NULL};

// Record buffer
#ifndef SDSIO_NO_BUFFER
static uint8_t   RecBuf[SDS_REC_MAX_RECORD_SIZE];
static uint8_t *pRecBuf = &RecBuf[0];
#else
static uint8_t   RecBuf[SDS_REC_MAX_RECORD_SIZE + SDSIO_HEADER_SIZE + SDSIO_TAIL_SIZE];
static uint8_t *pRecBuf = &RecBuf[SDSIO_HEADER_SIZE];
#endif

// Thread Id
static osThreadId_t sdsRecThreadId;

// Mutex Lock
static const osMutexAttr_t MutexLockAtr = {
  "Mutex_Lock",                         // Mutex name
  osMutexPrioInherit,                   // attr_bits
  NULL,                                 // Memory for control block
  0U                                    // Size for control block
};
static osMutexId_t MutexLockId;

// Event definitions
#define SDS_REC_EVENT_FLAG_MASK  ((1UL << SDS_REC_MAX_STREAMS) - 1)


// Helper functions

static inline void sdsRecLockCreate (void) {
  MutexLockId = osMutexNew(&MutexLockAtr);
}
static inline void sdsRecLockDelete (void) {
  osMutexDelete(MutexLockId);
}
static inline void sdsRecLock (void) {
  osMutexAcquire(MutexLockId, osWaitForever);
}
static inline void sdsRecUnLock (void) {
  osMutexRelease(MutexLockId);
}

static sdsRec_t * sdsRecAlloc (uint32_t *index) {
  sdsRec_t *rec = NULL;
  uint32_t n;

  for (n = 0U; n < SDS_REC_MAX_STREAMS; n++) {
    if (pRecStreams[n] == NULL) {
      rec = &RecStreams[n];
      if (index != NULL) {
        *index = n;
      }
      pRecStreams[n] = rec;
      break;
    }
  }
  return rec;
}

static void sdsRecFree (sdsRec_t *rec) {
  uint32_t n;

  if (rec != NULL) {
    for (n = 0U; n < SDS_REC_MAX_STREAMS; n++) {
      if (pRecStreams[n] == rec) {
        pRecStreams[n] = NULL;
        break;
      }
    }
  }
}

// Event callback
static void sdsRecEventCallback (sdsId_t id, uint32_t event, void *arg) {
  uint32_t flags = (uint32_t)arg;
  (void)id;
  (void)event;

  osThreadFlagsSet(sdsRecThreadId, flags);
}

// Recorder thread
static __NO_RETURN void sdsRecThread (void *arg) {
  sdsRec_t *rec;
  uint32_t flags, cnt, n;

  (void)arg;

  while (1) {
    flags = osThreadFlagsWait(SDS_REC_EVENT_FLAG_MASK, osFlagsWaitAny, osWaitForever);
    sdsRecLock();
    if ((flags & osFlagsError) == 0U) {
      for (n = 0U; n < SDS_REC_MAX_STREAMS; n++) {
        if (flags & (1U << n)) {
          rec = pRecStreams[n];
          if (rec != NULL) {
            while (sdsGetCount(rec->stream) >= rec->record_size) {
              cnt = sdsRead(rec->stream, pRecBuf, rec->record_size);
              sdsioWrite(rec->sdsio, pRecBuf, cnt);
            }
          }
        }
      }
    }
    sdsRecUnLock();
  }
}

// SDS Recorder functions

// Initialize recorder
int32_t sdsRecInit (sdsRecEvent_t event_cb) {
  int32_t ret = SDS_REC_ERROR;
  (void)event_cb;  // Not yet supported

  memset(pRecStreams, 0, sizeof(pRecStreams));

  sdsRecLockCreate();
  sdsRecThreadId = osThreadNew(sdsRecThread, NULL, NULL);
  if (sdsRecThreadId != NULL)  {
    ret = SDS_OK;
  }
  return ret;
}

// Uninitialize recorder
int32_t sdsRecUninit (void) {
  int32_t ret = SDS_ERROR;

  sdsRecLock();
  osThreadTerminate(sdsRecThreadId);
  sdsRecUnLock();
  sdsRecLockDelete();

  return ret;
}

// Open recorder stream
sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t record_size) {
  sdsRec_t *rec = NULL;
  uint32_t index;

  if ((name != NULL) && (buf != NULL) && (buf_size != 0U) && (record_size != 0U)) {
    sdsRecLock();
    rec = sdsRecAlloc(&index);
    if (rec != NULL) {
      rec->record_size = record_size;
      rec->stream = sdsOpen(buf, buf_size, 0U, record_size);
      if (rec->stream != NULL) {
        sdsRegisterEvents(rec->stream, sdsRecEventCallback, SDS_EVENT_DATA_HIGH, (void *)(1U << index));
      }
      rec->sdsio = sdsioOpen(name, sdsioModeWrite); 
      if ((rec->stream == NULL) || (rec->sdsio == NULL)) {
        if (rec->stream != NULL) {
          sdsClose(rec->stream);
          rec->stream = NULL;
        }
        if (rec->sdsio != NULL) {
          sdsioClose(rec->sdsio);
          rec->sdsio = NULL;
        }
        sdsRecFree(rec);
        rec = NULL;
      }
    }
    sdsRecUnLock();
  }
  return rec;
}

// Close recorder stream
int32_t sdsRecClose (sdsRecId_t id) {
  sdsRec_t *rec = id;
  int32_t  ret = SDS_ERROR;

  if (rec != NULL) {
    sdsRecLock();
    sdsClose(rec->stream);
    sdsioClose(rec->sdsio);
    sdsRecFree(rec);
    sdsRecUnLock(); 

    ret = SDS_OK;
  }
  return ret;
}

// Write data to recorder stream
uint32_t sdsRecWrite (sdsRecId_t id, const void *buf, uint32_t buf_size) {
  sdsRec_t *rec = id;
  uint32_t num = 0U;

  if ((rec != NULL) && (buf != NULL) && (buf_size != 0U)) {
    sdsRecLock();
    num = sdsWrite(rec->stream, buf, buf_size);
    sdsRecUnLock();
  }
  return num;
}
