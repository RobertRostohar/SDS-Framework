/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * Synchronous Data Stream
 */
#include <string.h>

#include "cmsis_os2.h"
#include "sds.h"

/// Configuration
#define SDS_MAX_STREAMS         4UL

/// Control block
typedef struct {
  sdsEvent_t event_cb;
  uint32_t   event_mask;
  void      *event_arg;

  void      *buf;
  uint32_t   buf_size;
  uint32_t   threshold_hi;
  uint32_t   threshold_low;
  uint32_t   in_cnt;
  uint32_t   out_cnt;
} sds_t;

/// Allocate SDS control blocks
sds_t stream_a[SDS_MAX_STREAMS] = {0};

/// Helper functions

static sds_t *sdsGetStream (void) {
  uint32_t i;
  sds_t *stream = NULL;

  for (i = 0U; i < SDS_MAX_STREAMS; i++) {
    if ((stream_a[i].buf == NULL) && (stream_a[i].buf_size = 0U)) {
      stream = &stream_a[i];
      break;
    }
  }
  return stream;
}

static void sdsReleaseStream (sds_t * stream) {
  memset(stream, 0, sizeof(sds_t));
}

/**
  Open stream.
*/
sdsId_t sdsOpen (void *buf, uint32_t buf_size, uint32_t threshold_low, uint32_t threshold_high) {
  sds_t *stream;

  stream = sdsGetStream();

  if (stream != NULL) {
    stream->buf           = buf;
    stream->buf_size      = buf_size;
    stream->threshold_low = threshold_low;
    stream->threshold_hi  = threshold_high;
  }
  return stream;
}

/**
  Close stream.
*/
int32_t sdsClose (sdsId_t id) {
  int32_t ret    = SDS_ERROR;
  sds_t *stream = id;

  if (stream != NULL) {
    sdsReleaseStream(stream);
    ret = SDS_OK;
  }
  return ret;
}

/**
  Register stream events.
*/
int32_t sdsRegisterEvents (sdsId_t id, sdsEvent_t event_cb, uint32_t event_mask, void *arg) {
  int32_t ret    = SDS_ERROR;
  sds_t *stream = id;

  if (stream != NULL) {
    stream->event_cb   = event_cb;
    stream->event_mask = event_mask;
    stream->event_arg  = arg;
    ret = SDS_OK;
  }
  return ret;
}

/**
  Write data to stream.
*/
uint32_t sdsWrite (sdsId_t id, void *buf, uint32_t buf_size) {
  uint32_t tmp_num, num = 0U;
  uint32_t empty, empty_new, offset;
  sds_t *stream = id;

  if ((stream != NULL) && (buf != NULL) && (buf_size != 0U)) {

    empty  = stream->buf_size - (stream->in_cnt - stream->out_cnt);
    offset = stream->in_cnt & (stream->buf_size - 1);

    if (empty >= buf_size) {
      num = buf_size;
    } else {
      // not enough space in buffer
      num = empty;
    }

    tmp_num = stream->buf_size - offset;
    if (num > tmp_num) {
      // buffer rollover
      memcpy(stream->buf + offset, buf, tmp_num);
      memcpy(stream->buf, buf + tmp_num, num - tmp_num);
    } else {
      memcpy(stream->buf + offset, buf, num);
    }
    stream->in_cnt += num;

    if ((stream->event_cb != NULL) && (stream->event_mask & SDS_EVENT_DATA_HIGH)) {
      empty_new = stream->buf_size - (stream->in_cnt - stream->out_cnt);
      if ((empty < stream->threshold_hi) && (empty_new >= stream->threshold_hi)) {
        stream->event_cb(stream, SDS_EVENT_DATA_HIGH, stream->event_arg);
      }
    }
  }
  return num;
}

/**
  Read data from stream.
*/
uint32_t sdsRead (sdsId_t id, void *buf, uint32_t buf_size) {
  uint32_t tmp_num, num = 0U;
  uint32_t full, full_new, offset;
  sds_t *stream = id;

  if ((stream != NULL) && (buf != NULL) && (buf_size != 0U)) {

    full   = stream->in_cnt - stream->out_cnt;
    offset = stream->out_cnt & (stream->buf_size - 1);

    if (full >= buf_size) {
      num = buf_size;
    } else {
      // not enough data available
      num = full;
    }

    tmp_num = stream->buf_size - offset;
    if (num > tmp_num) {
      // buffer rollover
      memcpy(buf, stream->buf + offset, tmp_num);
      memcpy(buf + offset, stream->buf, num - tmp_num);
    } else {
      memcpy(buf, stream->buf + offset, num);
    }
    stream->out_cnt += num;

    if ((stream->event_cb != NULL) && (stream->event_mask & SDS_EVENT_DATA_LOW)) {
      full = stream->in_cnt - stream->out_cnt;
      if ((full >= stream->threshold_hi) && (full_new < stream->threshold_hi)) {
        stream->event_cb(stream, SDS_EVENT_DATA_LOW, stream->event_arg);
      }
    }
  }
  return num;
}

/**
  Get data count in stream.
*/
uint32_t sdsGetCount (sdsId_t id) {
  uint32_t num = 0U;
  sds_t *stream = id;

  if (stream != NULL) {
    num = stream->in_cnt - stream->out_cnt;
  }
  return num;
}