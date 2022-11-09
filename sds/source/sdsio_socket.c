/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SDS Recorder interface - iotSocket (UDP)
 */
#include <string.h>

#include "cmsis_os2.h"
#include "iot_socket.h"

#include "sdsio.h"

// Configuration
#ifndef SERVER_IP
#define SERVER_IP               {0, 0, 0, 0}
#endif
#ifndef SERVER_PORT
#define SERVER_PORT             5000U
#endif

#ifndef SOCKET_RECEIVE_TOUT
#define SOCKET_RECEIVE_TOUT     500U
#endif
#ifndef SOCKET_RETRIES
#define SOCKET_RETRIES          3U
#endif

#ifndef SDSIO_MAX_STREAMS
#define SDSIO_MAX_STREAMS       4U
#endif
#ifndef SDSIO_NAME_MAX_LEN
#define SDSIO_NAME_MAX_LEN      16U
#endif

#ifndef SDSIO_NO_BUFFER
#define SDSIO_MAX_RECORD_SIZE   1024U
#else
#ifndef SDSIO_HEADER_SIZE
#define SDSIO_HEADER_SIZE       16U
#endif
#ifndef SDSIO_TAIL_SIZE
#define SDSIO_TAIL_SIZE         0U
#endif
#endif

/* SDSIO Socket Protocol
  Request:
    Header:
      byte 0  to 3:          packet idx
      byte 4  to 7:          cmd
      byte 8  to 11:         argument
      byte 12 to 15:         data_size
    Data:
      byte 16...:            data
  Response:
    Header:
      byte 0  to 3:          packet idx
      byte 4  to 7:          cmd
      byte 8  to 11:         argument
      byte 12 to 15:         data_size
    Data:
      byte 16...:            data
*/
typedef struct {
  uint32_t packet_idx;
  uint32_t command;
  uint32_t argument;
  uint32_t data_size;
} request_t, response_t;

// Commands
#define SDSIO_CMD_OPEN          1U
#define SDSIO_CMD_CLOSE         2U
#define SDSIO_CMD_WRITE         3U
#define SDSIO_CMD_READ          4U

#ifndef SDSIO_NO_BUFFER
static uint8_t  sdsio_buf_tx[sizeof(request_t)  + SDSIO_MAX_RECORD_SIZE];
static uint8_t  sdsio_buf_rx[sizeof(response_t) + SDSIO_MAX_RECORD_SIZE];
#else
#if (SDSIO_HEADER_SIZE < 16U)
#error "SDSIO_HEADER_SIZE must be equal or higher than 16!"
#endif
static uint8_t  sdsio_buf_tx[sizeof(request_t)  + SDSIO_NAME_MAX_LEN];
static uint8_t  sdsio_buf_rx[sizeof(response_t) + 4U];
#endif
static void    *sdsio_buf_req  = sdsio_buf_tx;
static void    *sdsio_buf_resp = sdsio_buf_rx;

static uint32_t sdsio_packet_idx = 0U;
static uint32_t sdsio_cnt        = 0U;

static const uint8_t ip[] = SERVER_IP;
static int32_t socket;

#ifdef SDSIO_LOCK
static osMutexId_t lock_id;
#endif

// Helper functions
#ifdef SDSIO_LOCK
static inline void sdsioLockCreate (void) {
  lock_id = osMutexNew(NULL);
}
static inline void sdsioLockDelete (void) {
  osMutexDelete(lock_id);
}
static inline void sdsioLock (void) {
  osMutexAcquire(lock_id, osWaitForever);
}
static inline void sdsioUnLock (void) {
  osMutexRelease(lock_id);
}
#else
static inline void sdsioLockCreate (void) {}
static inline void sdsioLockDelete (void) {}
static inline void sdsioLock       (void) {}
static inline void sdsioUnLock     (void) {}
#endif

/**
  \fn          uint32_t sdsioTransfer (void *req_buf, uint32_t req_buf_size, void *resp_buf, uint32_t *resp_buf_size)
  \brief       Send request and receive response
  \param[in]   req_buf        request buffer
  \param[in]   req_buf_size   size of request buffer in bytes
  \param[out]  resp_buf       response buffer
  \param[in]   resp_buf_size  size of response buffer in bytes
  \return      response size
*/
static uint32_t sdsioTransfer (void *req_buf, uint32_t req_buf_size, void *resp_buf, uint32_t resp_buf_size) { 
  request_t  *req  = (request_t  *)req_buf;
  response_t *resp = (response_t *)resp_buf;
  uint32_t    resp_valid = 0U;
  uint32_t    resp_size  = 0U;
  uint32_t    n, m;
  int32_t     num;

  req->packet_idx = sdsio_packet_idx;
  for (n = 0U; (n < SOCKET_RETRIES) && (resp_valid == 0U); n++) {
    // Send request packet
    if (iotSocketSendTo(socket, req_buf, req_buf_size, ip, sizeof(ip), SERVER_PORT) >= 0) {

      // Receive response packet
      for (m = 0U; m < SOCKET_RETRIES; m++) {
        num = iotSocketRecvFrom(socket, resp_buf, resp_buf_size, NULL, NULL,  NULL);
        if ((uint32_t)num >= sizeof(response_t)) {
          if (resp->packet_idx < req->packet_idx) {
            // Response belongs to an older request. Wait for a new response.
            continue;
          }
          // Packet idx, cmd and argument in the response must be equal as in request
          if ((memcmp(resp_buf, req_buf, 12U) == 0) &&
              ((uint32_t)num == (resp->data_size + sizeof(response_t)))) {
            resp_size  = (uint32_t)num;
            resp_valid = 1U;
          }
          break;
        }
      }
    }
  }
  if (resp_valid == 1U) {
    // Update packet index
     sdsio_packet_idx++;
  }

  return resp_size;
}

static int32_t sdsioInit (void) {
  int32_t  ret  = SDSIO_ERROR;
  uint32_t tout = SOCKET_RECEIVE_TOUT;

  sdsioLockCreate();
  socket = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_IPPROTO_UDP);
  if (socket >= 0) {
    if (iotSocketSetOpt(socket, IOT_SOCKET_SO_RCVTIMEO, &tout, sizeof(tout)) == 0) {
      ret = SDSIO_OK;
    }
  }
  return ret;
}

static int32_t sdsioUninit (void) {
  iotSocketClose(socket);
  sdsioLockDelete();
  return SDSIO_OK;
}

// SDS Recorder Interface functions

/**
  Open I/O stream
    Request:
      header: packet idx, SDSIO_CMD_OPEN, sdsioMode_t, data size = size of stream name
      data:   stream name (string limited to SDSIO_NAME_MAX_LEN)
    Response:
      header: packet idx, SDSIO_CMD_OPEN, sdsioMode_t, data size = 4
      data:   sdsio identifier (4 bytes)
*/
sdsioId_t sdsioOpen (const char *name, sdsioMode_t mode) {
  void       *req_data, *resp_data;
  uint32_t    resp_size;
  request_t  *req  = sdsio_buf_req;
  response_t *resp = sdsio_buf_resp;
  int32_t     stat = SDSIO_OK;
  uint32_t    sdsio_id = 0U;

  if ((name == NULL) || (strlen(name) >= SDSIO_NAME_MAX_LEN)) {
    stat = SDSIO_ERROR;
  }
  if ((stat == SDSIO_OK) && (sdsio_cnt == 0U)) {
    stat = sdsioInit();
  }
  sdsioLock();
  if (stat == SDSIO_OK) {
    req->command   = SDSIO_CMD_OPEN;
    req->argument  = mode;
    req->data_size = strlen(name) + 1U;

    req_data = (uint8_t *)req + sizeof(request_t);
    strcpy((char *)req_data, name);

    resp_size = sdsioTransfer(req, sizeof(request_t) + req->data_size, sdsio_buf_resp, sizeof(sdsio_buf_rx));

    if ((resp_size != 0U) && (resp->data_size == 4U)) {
      resp_data =  (uint8_t  *)resp + sizeof(request_t);
      sdsio_id  = *(uint32_t *)resp_data;
      sdsio_cnt++;
    } else {
      stat = SDSIO_ERROR;
    }
  }

  if ((stat == SDSIO_ERROR) && (sdsio_cnt == 0U)) {
    sdsioUninit();
  }

  sdsioUnLock();
  return (sdsioId_t)sdsio_id;
}

/**
  Close I/O stream.
  Request:
      header: packet idx, SDSIO_CMD_CLOSE, sdsio identifier, data size = 0
      data:   no data
    Response:
      header: packet idx, SDSIO_CMD_CLOSE, sdsio identifier, data size = 4
      data    0=Ok, else error (4 bytes)
*/
int32_t sdsioClose (sdsioId_t id) {  
  void       *resp_data;
  uint32_t    resp_size;
  request_t  *req  = sdsio_buf_req;
  response_t *resp = sdsio_buf_resp;
  int32_t     ret  = SDSIO_ERROR;

  sdsioLock();
  if (id != NULL) {
    req->command   = SDSIO_CMD_CLOSE;
    req->argument  = (uint32_t)id;
    req->data_size = 0U;

    resp_size = sdsioTransfer(req, sizeof(request_t) + req->data_size, sdsio_buf_resp, sizeof(sdsio_buf_rx));

    if ((resp_size != 0U) && (resp->data_size == 4U)) {
      resp_data = (uint8_t *)resp + sizeof(request_t);
      if (*(uint32_t *)resp_data == 0U) {
        ret = SDSIO_OK;
        sdsio_cnt--;
      }
    }
  }
  if (sdsio_cnt == 0U) {
    sdsioUninit();
  }
  sdsioUnLock();

  return ret;
}

/**
  Write data to I/O stream.
    Request:
      header: packet idx, SDSIO_CMD_WRITE, sdsio identifier, data size
      data:   data to be written
    Response:
      header: packet idx, SDSIO_CMD_WRITE, sdsio identifier, data size = 4
      data    number of bytes written
*/
uint32_t sdsioWrite (sdsioId_t id, const void *buf, uint32_t buf_size) {
  void       *req_data, *resp_data;
  uint32_t    resp_size;
  request_t  *req;
  response_t *resp = sdsio_buf_resp;
  uint32_t    num  = 0U;

  sdsioLock();
  if (id != NULL) {
#ifndef SDSIO_NO_BUFFER
    req = sdsio_buf_req;
#else
    req = (request_t *)((uint8_t *)buf - sizeof(request_t));
#endif
    req->command   = SDSIO_CMD_WRITE;
    req->argument  = (uint32_t)id;
    req->data_size = buf_size;

#ifndef SDSIO_NO_BUFFER
    req_data = (uint8_t *)req + sizeof(request_t);
    memcpy(req_data, buf, buf_size);
#endif

    resp_size = sdsioTransfer(req, sizeof(request_t) + req->data_size, sdsio_buf_resp, sizeof(sdsio_buf_rx));

    if ((resp_size != 0U) && (resp->data_size == 4U)) {
      resp_data = (uint8_t *)resp + sizeof(request_t);
      num = *(uint32_t *)resp_data;
    }
  }
  sdsioUnLock();
  return num;
}

/**
  Read data from I/O stream.
    Request:
      header: packet idx, SDSIO_CMD_READ, sdsio identifier, data size = 4
      data:   number of bytes to be read
    Response:
      header: packet idx, SDSIO_CMD_READ, sdsio identifier, data size = number of bytes read
      data    data read
*/
uint32_t sdsioRead (sdsioId_t id, void *buf, uint32_t buf_size) {
  void       *req_data, *resp_data;
  uint32_t    req_buf_size, resp_size, resp_buf_size;
  request_t  *req;
  response_t *resp;
  uint32_t    num = 0U;

  sdsioLock();
  if (id != NULL) {
    req = sdsio_buf_req;
    req->command   = SDSIO_CMD_READ;
    req->argument  = (uint32_t)id;
    req->data_size = 4U;

#ifndef SDSIO_NO_BUFFER
    resp = sdsio_buf_resp;
    if (buf_size > (sizeof(sdsio_buf_rx) - sizeof(response_t))) {
      resp_buf_size = sizeof(sdsio_buf_rx);
    } else {
      resp_buf_size = buf_size + sizeof(response_t);
    }
#else
    resp = (response_t *)((uint8_t *)buf - sizeof(response_t));
    resp_buf_size = buf_size + sizeof(response_t);
#endif

    req_buf_size = resp_buf_size - sizeof(response_t);
    req_data = (uint8_t *)req + sizeof(request_t);
    memcpy(req_data, &req_buf_size, sizeof(req_buf_size));

    resp_size = sdsioTransfer(req, sizeof(request_t) + req->data_size, resp, resp_buf_size);

    if (resp_size != 0U) {
      num = resp->data_size;
#ifndef SDSIO_NO_BUFFER
      if (num != 0U) {
        resp_data = (uint8_t *)resp + sizeof(response_t);
        memcpy(buf, resp_data, num);
      }
#endif
    }
  }
  sdsioUnLock();
  return num;
}
