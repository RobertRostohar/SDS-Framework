/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// Synchronous Data Stream (SDS)
#include <string.h>

#include "cmsis_compiler.h"
#include "sds.h"

// Configuration
#ifndef SDS_MAX_STREAMS
#define SDS_MAX_STREAMS         16U
#endif

// Control block
typedef struct {
  sdsEvent_t event_cb;
  uint32_t   event_mask;
  void      *event_arg;
  uint8_t   *buf;
  uint32_t   buf_size;
  uint32_t   threshold_high;
  uint32_t   threshold_low;
  uint32_t   cnt_in;
  uint32_t   cnt_out;
  uint32_t   idx_in;
  uint32_t   idx_out;
} sds_t;

static sds_t   Streams[SDS_MAX_STREAMS] = {0};
static sds_t *pStreams[SDS_MAX_STREAMS] = {NULL};

// Helper functions

#ifndef EXCLUSIVE_ACCESS
#if   ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) || \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)) || \
       (defined(__ARM_ARCH_8M_BASE__)   && (__ARM_ARCH_8M_BASE__   != 0)) || \
       (defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) || \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)))
#define EXCLUSIVE_ACCESS        1
#else
#define EXCLUSIVE_ACCESS        0
#endif
#endif

// Atomic Access Operation: Write 32-bit value to the memory, if exsisting value in the memory is zero
//  Return: New value in the memory or 0 if exsisting value in the memory is not zero
#if (EXCLUSIVE_ACCESS == 0)
__STATIC_INLINE uint32_t atomic_wr32_z (uint32_t *mem, uint32_t val) {
  uint32_t primask = __get_PRIMASK();
  uint32_t ret = 0U;

  __disable_irq();
  if (*mem == NULL) {
    *mem = val;
    ret = val;
  }
  if (primask == 0U) {
    __enable_irq();
  }

  return ret;
}
#else
__STATIC_INLINE uint32_t atomic_wr32_z (uint32_t *mem, uint32_t val) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t res, ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[res],[%[mem]]\n\t"
    "cbz   %[res],2f\n\t"
    "clrex\n\t"
    "mov  %[ret],#0\n\t"
    "b     3f\n"
  "2:\n\t"
    "strex %[res],%[val],[%[mem]]\n\t"
    "ldr   %[ret],[%[mem]]\n\t"
    "cbz   %[res],3f\n\t"
    "b     1b\n"
  "3:"
  : [ret] "=&l" (ret),
    [res] "=&l" (res)
  : [mem] "l"   (mem),
    [val] "l"   (val)
  : "cc", "memory"
  );
  return ret;
}
#endif


static sds_t *sdsAlloc (void) {
  sds_t *stream = NULL;
  uint32_t n;

  for (n = 0U; n < SDS_MAX_STREAMS; n++) {
    if (atomic_wr32_z((uint32_t *)&pStreams[n], (uint32_t)&Streams[n]) != 0U) {
      stream = &Streams[n];
      break;
    }
  }
  return stream;
}

static void sdsFree (sds_t *stream) {
  uint32_t n;

  if (stream != NULL) {
    for (n = 0U; n < SDS_MAX_STREAMS; n++) {
      if (pStreams[n] == stream) {
        pStreams[n] = NULL;
        break;
      }
    }
  }
}

// Open stream
sdsId_t sdsOpen (void *buf, uint32_t buf_size, uint32_t threshold_low, uint32_t threshold_high) {
  sds_t *stream = NULL;

  // Buffer pointer needs to be valid
  if ((buf != NULL) && (buf_size != 0U)) {
    stream = sdsAlloc();
    if (stream != NULL) {
      memset(stream, 0, sizeof(sds_t));
      stream->buf            = buf;
      stream->buf_size       = buf_size;
      stream->threshold_low  = threshold_low;
      stream->threshold_high = threshold_high;
    }
  }
  return stream;
}

// Close stream
int32_t sdsClose (sdsId_t id) {
  sds_t *stream = id;
  int32_t ret = SDS_ERROR;

  if (stream != NULL) {
    sdsFree(stream);
    ret = SDS_OK;
  }
  return ret;
}

// Register stream events
int32_t sdsRegisterEvents (sdsId_t id, sdsEvent_t event_cb, uint32_t event_mask, void *event_arg) {
  sds_t *stream = id;
  int32_t ret = SDS_ERROR;

  if ((stream != NULL) && (event_cb != NULL) && (event_mask != 0U)) {
    stream->event_cb   = event_cb;
    stream->event_mask = event_mask;
    stream->event_arg  = event_arg;
    ret = SDS_OK;
  }
  return ret;
}

// Write data to stream
uint32_t sdsWrite (sdsId_t id, const void *buf, uint32_t buf_size) {
  sds_t *stream = id;
  uint32_t num = 0U;
  uint32_t cnt_free, cnt_used, cnt_used_new, cnt_limit;

  if ((stream != NULL) && (buf != NULL) && (buf_size != 0U)) {

    cnt_used = stream->cnt_in - stream->cnt_out;
    cnt_free = stream->buf_size - cnt_used;

    if (buf_size < cnt_free) {
      num = buf_size;
    } else {
      // not enough space in buffer
      num = cnt_free;
    }

    cnt_limit = stream->buf_size - stream->idx_in;
    if (num > cnt_limit) {
      // buffer rollover
      memcpy(stream->buf + stream->idx_in, buf, cnt_limit);
      memcpy(stream->buf, (const uint8_t *)buf + cnt_limit, num - cnt_limit);
      stream->idx_in = num - cnt_limit;
    } else {
      memcpy(stream->buf + stream->idx_in, buf, num);
      stream->idx_in += num;
    }
    stream->cnt_in += num;

    if ((stream->event_cb != NULL) && (stream->event_mask & SDS_EVENT_DATA_HIGH)) {
      cnt_used_new = stream->cnt_in - stream->cnt_out;
      if ((cnt_used < stream->threshold_high) && (cnt_used_new >= stream->threshold_high)) {
        stream->event_cb(stream, SDS_EVENT_DATA_HIGH, stream->event_arg);
      }
    }
  }
  return num;
}

// Read data from stream
uint32_t sdsRead (sdsId_t id, void *buf, uint32_t buf_size) {
  sds_t *stream = id;
  uint32_t num = 0U;
  uint32_t cnt_used, cnt_used_new, cnt_limit;

  if ((stream != NULL) && (buf != NULL) && (buf_size != 0U)) {

    cnt_used = stream->cnt_in - stream->cnt_out;

    if (buf_size < cnt_used) {
      num = buf_size;
    } else {
      // not enough data available
      num = cnt_used;
    }

    cnt_limit = stream->buf_size - stream->idx_out;
    if (num > cnt_limit) {
      // buffer rollover
      memcpy(buf, stream->buf + stream->idx_out, cnt_limit);
      memcpy((uint8_t *)buf + cnt_limit, stream->buf, num - cnt_limit);
      stream->idx_out = num - cnt_limit;
    } else {
      memcpy(buf, stream->buf + stream->idx_out, num);
      stream->idx_out += num;
    }
    stream->cnt_out += num;

    if ((stream->event_cb != NULL) && (stream->event_mask & SDS_EVENT_DATA_LOW)) {
      cnt_used_new = stream->cnt_in - stream->cnt_out;
      if ((cnt_used >= stream->threshold_low) && (cnt_used_new < stream->threshold_low)) {
        stream->event_cb(stream, SDS_EVENT_DATA_LOW, stream->event_arg);
      }
    }
  }
  return num;
}

// Clear stream data
int32_t sdsClear (sdsId_t id) {
  sds_t *stream = id;
  uint32_t cnt_used, cnt_limit;

  cnt_used = stream->cnt_in - stream->cnt_out;
  cnt_limit = stream->buf_size - stream->idx_out;
  if (cnt_used > cnt_limit) {
    // buffer rollover
    stream->idx_out = cnt_used - cnt_limit;
  } else {
    stream->idx_out += cnt_used;
  }
  stream->cnt_out += cnt_used;

  return SDS_OK;
}

// Get data count in stream
uint32_t sdsGetCount (sdsId_t id) {
  sds_t *stream = id;
  uint32_t num = 0U;

  if (stream != NULL) {
    num = stream->cnt_in - stream->cnt_out;
  }
  return num;
}
