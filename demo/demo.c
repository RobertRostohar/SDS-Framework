/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */
#include <stdio.h>

#include "cmsis_vio.h"
#include "cmsis_os2.h"

#include "sds.h"
#ifdef RECORDER_ENABLED
#include "sds_rec.h"
#endif

#include "sensor_drv.h"
#include "sensor_config.h"

// Configuration
#ifndef SDS_BUF_SIZE_ACCELEROMETER
#define SDS_BUF_SIZE_ACCELEROMETER          4096U
#endif
#ifndef SDS_BUF_SIZE_TEMPERATURE_SENSOR
#define SDS_BUF_SIZE_TEMPERATURE_SENSOR     128U
#endif
#ifndef SDS_THRESHOLD_ACCELEROMETER
#define SDS_THRESHOLD_ACCELEROMETER         624U
#endif
#ifndef SDS_THRESHOLD_TEMPERATURE_SENSOR
#define SDS_THRESHOLD_TEMPERATURE_SENSOR    4U
#endif

#ifdef RECORDER_ENABLED
#ifndef REC_BUF_SIZE_ACCELEROMETER
#define REC_BUF_SIZE_ACCELEROMETER          8192U
#endif
#ifndef REC_BUF_SIZE_TEMPERATURE_SENSOR
#define REC_BUF_SIZE_TEMPERATURE_SENSOR     256U
#endif
#ifndef REC_RECORD_SIZE_ACCELEROMETER
#define REC_RECORD_SIZE_ACCELEROMETER       1020U
#endif
#ifndef REC_RECORD_SIZE_TEMPERATURE_SENSOR
#define REC_RECORD_SIZE_TEMPERATURE_SENSOR  16U
#endif
#endif

#ifndef SENSOR_POLLING_INTERVAL
#define SENSOR_POLLING_INTERVAL             5U  /* 5ms */
#endif

// Sensor identifiers
static sensorId_t sensorId_accelerometer              = NULL;
static sensorId_t sensorId_temperatureSensor          = NULL;

// Sensor configuration
static sensorConfig_t *sensorConfig_accelerometer     = NULL;
static sensorConfig_t *sensorConfig_temperatureSensor = NULL;

// SDS identifiers
static sdsId_t sdsId_accelerometer                    = NULL;
static sdsId_t sdsId_temperatureSensor                = NULL;

// SDS buffers
static uint8_t sdsBuf_accelerometer[SDS_BUF_SIZE_ACCELEROMETER];
static uint8_t sdsBuf_temperatureSensor[SDS_BUF_SIZE_TEMPERATURE_SENSOR];

#ifdef RECORDER_ENABLED
// Recorder identifiers
static sdsId_t recId_accelerometer     = NULL;
static sdsId_t recId_temperatureSensor = NULL;

// Recorder buffers
static uint8_t recBuf_accelerometer[REC_BUF_SIZE_ACCELEROMETER];
static uint8_t recBuf_temperatureSensor[REC_BUF_SIZE_TEMPERATURE_SENSOR];
#endif

// Thread identifiers
static osThreadId_t thrId_demo         = NULL;
static osThreadId_t thrId_read_sensors = NULL;

#define EVENT_DATA_ACCELEROMETER        (1U << 0)
#define EVENT_DATA_TEMPERATURE_SENSOR   (1U << 1)
#define EVENT_BUTTON                    (1U << 2)
#define EVENT_DATA_MASK                 (EVENT_DATA_ACCELEROMETER | EVENT_DATA_TEMPERATURE_SENSOR)
#define EVENT_MASK                      (EVENT_DATA_MASK | EVENT_BUTTON)

// Sensor event callback
static void sensor_event_callback (sensorId_t id, uint32_t event) {

  if ((event & SENSOR_EVENT_DATA) != 0U) {
    if (id == sensorId_accelerometer) {
      osThreadFlagsSet(thrId_read_sensors, EVENT_DATA_ACCELEROMETER);
    }
    if (id == sensorId_temperatureSensor) {
      osThreadFlagsSet(thrId_read_sensors, EVENT_DATA_TEMPERATURE_SENSOR);
    }
  }
}

// Read sensor thread
static __NO_RETURN void read_sensors (void *argument) {
  uint32_t num, flags, buf_size;
  uint8_t  buf[6];
  (void)   argument;

  for (;;) {
    flags = osThreadFlagsWait(EVENT_DATA_MASK, osFlagsWaitAny, SENSOR_POLLING_INTERVAL);

    if ((((flags & osFlagsError) == 0U) && ((flags & EVENT_DATA_ACCELEROMETER) != 0U)) ||
          (flags == osFlagsErrorTimeout)) {

      if (sensorGetStatus(sensorId_accelerometer).active != 0U) {
        num = sensorReadSamples(sensorId_accelerometer, 1U, buf, sizeof(buf));
        if (num != 0U) {
          buf_size = num * sensorConfig_accelerometer->sample_size;
          sdsWrite(sdsId_accelerometer, buf, buf_size);
#ifdef RECORDER_ENABLED
          sdsRecWrite(recId_accelerometer, buf, buf_size);
#endif
        }
      }
    }

    if ((((flags & osFlagsError) == 0U) && ((flags & EVENT_DATA_TEMPERATURE_SENSOR) != 0U)) ||
          (flags == osFlagsErrorTimeout)) {

      if (sensorGetStatus(sensorId_temperatureSensor).active != 0U) {
        num = sensorReadSamples(sensorId_temperatureSensor, 1U, buf, sizeof(buf));
        if (num != 0U) {
          buf_size = num * sensorConfig_temperatureSensor->sample_size;
          sdsWrite(sdsId_temperatureSensor, buf, buf_size);
#ifdef RECORDER_ENABLED
          sdsRecWrite(recId_temperatureSensor, buf, buf_size);
#endif
        }
      }
    }
  }
}

// Button thread
static __NO_RETURN void button (void *argument) {
  uint32_t value, value_last = 0U;
  (void)   argument;

  for (;;) {
    // Monitor user button
    value = vioGetSignal(vioBUTTON0);
    if (value != value_last) {
      value_last = value;
      if (value == vioBUTTON0) {
        // Button pressed
        osThreadFlagsSet(thrId_demo, EVENT_BUTTON);
      }
    }
    osDelay(100U);
  }
}

// SDS event callback
static void sds_event_callback (sdsId_t id, uint32_t event, void *arg) {
  (void)arg;

  if ((event & SDS_EVENT_DATA_HIGH) != 0U) {
    if (id == sdsId_accelerometer) {
      osThreadFlagsSet(thrId_demo, EVENT_DATA_ACCELEROMETER);
    }
    if (id == sdsId_temperatureSensor) {
      osThreadFlagsSet(thrId_demo, EVENT_DATA_TEMPERATURE_SENSOR);
    }
  }
}

// Sensor Demo
void __NO_RETURN demo(void) {
  uint32_t  n, num, flags;
  uint32_t  buf[2];
  uint16_t *data_u16 = (uint16_t *)buf;
  float    *data_f   = (float *)buf;

  thrId_demo = osThreadGetId();

  // Get sensor identifier
  sensorId_accelerometer     = sensorGetId("Accelerometer");
  sensorId_temperatureSensor = sensorGetId("Temperature");

  // Get sensor configuration
  sensorConfig_accelerometer     = sensorGetConfig(sensorId_accelerometer);
  sensorConfig_temperatureSensor = sensorGetConfig(sensorId_temperatureSensor);

  // Register sensor event
  sensorRegisterEvents(sensorId_accelerometer,     sensor_event_callback, SENSOR_EVENT_DATA);
  sensorRegisterEvents(sensorId_temperatureSensor, sensor_event_callback, SENSOR_EVENT_DATA);

  // Open SDS
  sdsId_accelerometer     = sdsOpen(sdsBuf_accelerometer,
                                    sizeof(sdsBuf_accelerometer),
                                    0U, SDS_THRESHOLD_ACCELEROMETER);

  sdsId_temperatureSensor = sdsOpen(sdsBuf_temperatureSensor,
                                    sizeof(sdsBuf_temperatureSensor),
                                    0U, SDS_THRESHOLD_TEMPERATURE_SENSOR);

  // Register SDS events
  sdsRegisterEvents(sdsId_accelerometer,     sds_event_callback, SDS_EVENT_DATA_HIGH, NULL);
  sdsRegisterEvents(sdsId_temperatureSensor, sds_event_callback, SDS_EVENT_DATA_HIGH, NULL);

#ifdef RECORDER_ENABLED
  // Initialize recorder
  sdsRecInit();
#endif

  // Create sensor thread
  thrId_read_sensors = osThreadNew(read_sensors, NULL, NULL);

  // Create button thread
  osThreadNew(button, NULL, NULL);

  for(;;) {
    flags = osThreadFlagsWait(EVENT_MASK, osFlagsWaitAny, osWaitForever);
    if ((flags & osFlagsError) == 0U) {

      // Button pressed event
      if (flags & EVENT_BUTTON) {
        printf("Button pressed\r\n");

        if (sensorGetStatus(sensorId_accelerometer).active == 0U) {
          sdsClear(sdsId_accelerometer);
#ifdef RECORDER_ENABLED
          // Open Recorder
          recId_accelerometer = sdsRecOpen("Accelerometer",
                                           recBuf_accelerometer,
                                           sizeof(recBuf_accelerometer),
                                           REC_RECORD_SIZE_ACCELEROMETER);
#endif
          sensorEnable(sensorId_accelerometer);
          printf("Accelerometer enabled\r\n");
        } else {
          sensorDisable(sensorId_accelerometer);
#ifdef RECORDER_ENABLED
          // Close Recorder
          sdsRecClose(recId_accelerometer);
          recId_accelerometer = NULL;
#endif
          printf("Accelerometer disabled\r\n");
        }

        if (sensorGetStatus(sensorId_temperatureSensor).active == 0U) {
          sdsClear(sdsId_temperatureSensor);
#ifdef RECORDER_ENABLED
          // Open Recorder;
          recId_temperatureSensor = sdsRecOpen("Temperature",
                                               recBuf_temperatureSensor,
                                               sizeof(recBuf_temperatureSensor),
                                               REC_RECORD_SIZE_TEMPERATURE_SENSOR);
#endif
          sensorEnable(sensorId_temperatureSensor);
          printf("Temperature sensor enabled\r\n");
        } else {
          sensorDisable(sensorId_temperatureSensor);
#ifdef RECORDER_ENABLED
          // Close Recorder
          sdsRecClose(recId_temperatureSensor);
          recId_temperatureSensor = NULL;
#endif
          printf("Temperature sensor disabled\r\n");
        }
      }

      // Accelerometer data event
      if ((flags & EVENT_DATA_ACCELEROMETER) != 0U) {

        for (n = 0U; n < (SDS_THRESHOLD_ACCELEROMETER / sensorConfig_accelerometer->sample_size); n++) {
          num = sdsRead(sdsId_accelerometer, buf, sensorConfig_accelerometer->sample_size);
          if (num == sensorConfig_accelerometer->sample_size) {
            printf("%s: x=%i, y=%i, z=%i\r\n",sensorConfig_accelerometer->name,
                                              data_u16[0], data_u16[1], data_u16[2]);
          }
        }
      }

      // Temperature sensor data event
      if ((flags & EVENT_DATA_TEMPERATURE_SENSOR) != 0U) {

        for (n = 0U; n < (SDS_THRESHOLD_TEMPERATURE_SENSOR / sensorConfig_temperatureSensor->sample_size); n++) {
          num = sdsRead(sdsId_temperatureSensor, buf, sensorConfig_temperatureSensor->sample_size);
          if (num == sensorConfig_temperatureSensor->sample_size) {
            printf("%s: value=%f\r\n", sensorConfig_temperatureSensor->name, (double)*data_f);
          }
        }
      }
    }
  }
}
