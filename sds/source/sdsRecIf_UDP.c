/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SDS Recorder interface - iotSocket (UPD)
 */
#include <string.h>

#include "cmsis_os2.h"
#include "iot_socket.h"
#include "sds.h"
#include "sdsRecIf.h"

/// Configuration
#define IP                      {192, 168, 4, 100}
#define PORT                     5000
#define SOCKET_RECEIVE_TOUT      5000
#define SOCKET_RETRIES           3

#define SDS_REC_IF_MAX_STREAMS   4UL
#define SDS_REC_IF_NAME_MAX_LEN  15+1

static int32_t socket;

#define OK_RESPONSE             "OK"
#define START_CMD               "Start sdsRecorder"
#define STOP_CMD                "Stop sdsRecorder"

/// Packet header
typedef struct {
  char              name[SDS_REC_IF_NAME_MAX_LEN];
  uint32_t          idx;
} packet_header_t;

/// Control block
typedef struct {
  uint32_t          idx;
  packet_header_t   header;
  uint32_t          record_size;
  void             *sdsRecId;
} sdsRecIf_t;

/// Allocate SDS Recorder control blocks
sdsRecIf_t recIf_a[SDS_REC_IF_MAX_STREAMS] = {0};

uint8_t UDP_BufTx[1500];
uint8_t UDP_BufRx[64];

static const uint8_t ip[] = IP;

/// Helper functions

static sdsRecIf_t *sdsRecGetInterface (void) {
  uint32_t i;
  sdsRecIf_t *rec = NULL;

  for (i = 0U; i < SDS_REC_IF_MAX_STREAMS; i++) {
    if (recIf_a[i].header.name[0] == 0) {
      rec = &recIf_a[i];
      rec->idx = i;
      break;
    }
  }
  return rec;
}

static void sdsRecReleaseInterface (sdsRecIf_t *recIf) {
  memset(recIf, 0, sizeof(sdsRecIf_t));
}

static int32_t sdsRecIfSend (uint8_t *buf, uint32_t sz) {
  uint32_t i;
  int32_t ret = SDS_REC_IF_ERROR;

  for (i = 0U; i < SOCKET_RETRIES; i++) {
    if (iotSocketSendTo(socket, buf, sz, ip, sizeof(ip), PORT) >= 0) {
      memset(UDP_BufRx, 0, sizeof(UDP_BufRx));
      if (iotSocketRecvFrom(socket, UDP_BufRx, sizeof(UDP_BufRx), NULL, NULL,  NULL) >= 0) {
        if (strcmp((char *)UDP_BufRx, OK_RESPONSE) == 0) {
          ret = SDS_REC_IF_OK;
          break;
        }
      }
    }
  }
  return ret;
}

/// SDS Recorder Interface functions

/**
  Initialize recorder interface
*/
int32_t sdsRecIfInit (void) {
  int32_t ret = SDS_REC_IF_ERROR;
  uint32_t tout = SOCKET_RECEIVE_TOUT;

  socket = iotSocketCreate(IOT_SOCKET_AF_INET, IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_IPPROTO_UDP);
  if (socket  >= 0) {
    iotSocketSetOpt(socket, IOT_SOCKET_SO_RCVTIMEO, &tout, sizeof(tout));
    ret = sdsRecIfSend((uint8_t *)START_CMD, sizeof(START_CMD));
  }
  return ret;
}

/**
  Uninitialize recorder.
*/
int32_t sdsRecUninit (void) {
  sdsRecIfSend((uint8_t *)STOP_CMD, sizeof(STOP_CMD));
  iotSocketClose(socket);
  return SDS_REC_IF_OK;
}

/**
  Open recorder stream
*/
sdsRecIfId_t sdsRecIfOpen (const char *name, uint32_t record_size, void *arg) {
  sdsRecIf_t *recIf = NULL;

  if ((strlen(name) < SDS_REC_IF_NAME_MAX_LEN) &&
      (record_size <= (sizeof(UDP_BufTx) - sizeof(packet_header_t)))) {
    recIf = sdsRecGetInterface();
    if (recIf != NULL) {
      strcpy((char *)recIf->header.name, name);
      recIf->record_size = record_size;
      recIf->sdsRecId = arg;
    }
  }
  return recIf;
}

/**
  Close recorder stream.
*/
int32_t sdsRecIfClose (sdsRecIfId_t id) {
  int32_t     ret = SDS_REC_IF_ERROR;
  sdsRecIf_t *recIf = id;

  if (recIf != NULL) {
    memset(recIf, 0, sizeof(sdsRecIf_t));
    ret = SDS_REC_IF_OK;
  }

  return ret;
}

/**
  Notify recorder interface that data is available for transfer.
*/
int32_t sdsRecIfDataAvailable (sdsRecIfId_t id) {
  int32_t ret = SDS_REC_IF_ERROR;
  sdsRecIf_t *recIf = id;;

  if (recIf != NULL) {
    // Copy packet header
    memcpy(UDP_BufTx, &recIf->header, sizeof(packet_header_t));

    // Read data
    if (sdsRecRead(recIf->sdsRecId, (UDP_BufTx + sizeof(packet_header_t)), recIf->record_size) == recIf->record_size) {
      ret = sdsRecIfSend(UDP_BufTx, recIf->record_size + sizeof(packet_header_t));
    }
  }
  return ret;
}
