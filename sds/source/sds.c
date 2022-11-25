/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// Synchronous Data Stream (SDS)

#include <string.h>

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

static sds_t *sdsAlloc (void) {
  sds_t *stream = NULL;
  uint32_t n;

  for (n = 0U; n < SDS_MAX_STREAMS; n++) {
    if (pStreams[n] == NULL) {
      stream = &Streams[n];
      pStreams[n] = stream;
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

// Get data count in stream
uint32_t sdsGetCount (sdsId_t id) {
  sds_t *stream = id;
  uint32_t num = 0U;

  if (stream != NULL) {
    num = stream->cnt_in - stream->cnt_out;
  }
  return num;
}
