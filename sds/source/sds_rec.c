/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
  uint32_t    buf_size;
  uint32_t    flag_mask;
  sdsId_t     stream;
  sdsioId_t   sdsio;
} sdsRec_t;

static sdsRec_t   RecStreams[SDS_REC_MAX_STREAMS] = {0};
static sdsRec_t *pRecStreams[SDS_REC_MAX_STREAMS] = {NULL};

// Record header
typedef struct {
  uint32_t    timestamp;        // Timestamp in milliseconds
  uint32_t    data_size;        // Data size in bytes
} RecHead_t;

// Record buffer
#ifndef SDSIO_NO_BUFFER
static uint8_t   RecBuf[SDS_REC_MAX_RECORD_SIZE];
static uint8_t *pRecBuf = &RecBuf[0];
#else
static uint8_t   RecBuf[SDS_REC_MAX_RECORD_SIZE + SDSIO_HEADER_SIZE + SDSIO_TAIL_SIZE];
static uint8_t *pRecBuf = &RecBuf[SDSIO_HEADER_SIZE];
#endif

// Event callback
static sdsRecEvent_t sdsRecEvent = NULL;

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
            cnt = sdsRead(rec->stream, pRecBuf, SDS_REC_MAX_RECORD_SIZE);
            if (cnt != 0U) {
              if (sdsioWrite(rec->sdsio, pRecBuf, cnt) != cnt) {
                if (sdsRecEvent != NULL) {
                  sdsRecEvent(rec, SDS_REC_EVENT_IO_ERROR);
                }
              }
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

  memset(pRecStreams, 0, sizeof(pRecStreams));

  sdsRecLockCreate();
  sdsRecThreadId = osThreadNew(sdsRecThread, NULL, NULL);
  if (sdsRecThreadId != NULL)  {
    sdsRecEvent = event_cb;
    ret = SDS_OK;
  }
  return ret;
}

// Uninitialize recorder
int32_t sdsRecUninit (void) {
  int32_t ret = SDS_ERROR;

  sdsRecLock();
  osThreadTerminate(sdsRecThreadId);
  sdsRecEvent = NULL;
  sdsRecUnLock();
  sdsRecLockDelete();

  return ret;
}

// Open recorder stream
sdsRecId_t sdsRecOpen (const char *name, void *buf, uint32_t buf_size, uint32_t io_threshold) {
  sdsRec_t *rec = NULL;
  uint32_t index;

  if ((name != NULL) && (buf != NULL) && (buf_size != 0U) &&
      (buf_size <= SDS_REC_MAX_RECORD_SIZE) && (io_threshold <= buf_size)) {

    sdsRecLock();
    rec = sdsRecAlloc(&index);
    if (rec != NULL) {
      rec->buf_size = buf_size;
      rec->flag_mask = 0U;
      rec->stream = sdsOpen(buf, buf_size, 0U, io_threshold);
      rec->sdsio = sdsioOpen(name, sdsioModeWrite);
      if (rec->stream != NULL) {
        if (io_threshold != 0U) {
          sdsRegisterEvents(rec->stream, sdsRecEventCallback, SDS_EVENT_DATA_HIGH, (void *)(1U << index));
        } else {
          rec->flag_mask = 1U << index;
        }
      }
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
  uint32_t cnt;
  int32_t  ret = SDS_ERROR;

  if (rec != NULL) {
    sdsRecLock();
    cnt = sdsRead(rec->stream, pRecBuf, SDS_REC_MAX_RECORD_SIZE);
    if (cnt != 0U) {
      sdsioWrite(rec->sdsio, pRecBuf, cnt);
    }
    sdsClose(rec->stream);
    sdsioClose(rec->sdsio);
    sdsRecFree(rec);
    sdsRecUnLock(); 

    ret = SDS_OK;
  }
  return ret;
}

// Write data to recorder stream
uint32_t sdsRecWrite (sdsRecId_t id, uint32_t timestamp, const void *buf, uint32_t buf_size) {
  sdsRec_t *rec = id;
  RecHead_t rec_head;
  uint32_t num = 0U;

  if ((rec != NULL) && (buf != NULL) && (buf_size != 0U)) {
    sdsRecLock();
    if ((buf_size + sizeof(RecHead_t)) <= (rec->buf_size -  sdsGetCount(rec->stream))) {
      // Write record to the stream: timestamp, data size, data
      rec_head.timestamp = timestamp;
      rec_head.data_size = buf_size;
      if (sdsWrite(rec->stream, &rec_head, sizeof(RecHead_t)) == sizeof(RecHead_t)) {
        num = sdsWrite(rec->stream, buf, buf_size);
        if (rec->flag_mask != 0U) {
          osThreadFlagsSet(sdsRecThreadId, rec->flag_mask);
        }
      }
    }
    sdsRecUnLock();
  }
  return num;
}
