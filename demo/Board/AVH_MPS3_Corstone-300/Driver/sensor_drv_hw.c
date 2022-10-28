/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

// Sensor driver for VSI (Virtual Streaming Interface) on AVH (Arm Virtual Hardware)

#include <string.h>

#include "sensor_config.h"
#include "sensor_drv.h"
#include "sensor_drv_hw.h"

#include "RTE_Components.h"
#include CMSIS_device_header

#include "arm_vsi.h"

// VSI IRQ numbers
#define ARM_VSI0_IRQn   224     // VSI 0 Interrupt
#define ARM_VSI1_IRQn   225     // VSI 1 Interrupt
#define ARM_VSI2_IRQn   226     // VSI 2 Interrupt
#define ARM_VSI3_IRQn   227     // VSI 3 Interrupt
#define ARM_VSI4_IRQn   228     // VSI 4 Interrupt
#define ARM_VSI5_IRQn   229     // VSI 5 Interrupt
#define ARM_VSI6_IRQn   230     // VSI 6 Interrupt
#define ARM_VSI7_IRQn   231     // VSI 7 Interrupt


// VSI user registers
#define CONTROL                 Regs[0] // Control: 1=Enabled, 0=Disabled
#define STATUS                  Regs[1] // Status: Bit0=Overflow
#define SENSOR_NAME_LEN         Regs[2] // Sensor name length
#define SENSOR_NAME_CHAR        Regs[3] // Sensor name character
#define SENSOR_NAME_VALID       Regs[4] // Sensor name valid flag
#define SAMPLE_SIZE             Regs[5] // Sample size in bytes
#define SAMPLE_COUNT            Regs[6] // Sample data count
#define SAMPLE_PORT             Regs[7] // Sample data port
#define DATA_THRESHOLD          Regs[8] // Data event threshold in number of samples
#define FIFO_SIZE               Regs[9] // Sample FIFO size in bytes

// Control register definitions
#define CONTROL_ENABLE_Pos      0U                              // CONTROL: ENABLE Position
#define CONTROL_ENABLE_Msk      (1UL << CONTROL_ENABLE_Pos)     // CONTROL: ENABLE Mask
#define CONTROL_DMA_Pos         1U                              // CONTROL: DMA Position
#define CONTROL_DMA_Msk         (1UL << CONTROL_DMA_Pos)        // CONTROL: DMA Mask

// Status register definitions
#define STATUS_OVERFLOW_Pos     0U                              // STATUS: OVERFLOW Position
#define STATUS_OVERFLOW_Msk     (1UL << STATUS_OVERFLOW_Pos)    // STATUS: OVERFLOW Mask


// Sensor0 using VSI0
#ifdef SENSOR0_NAME

// Registered event variables
static sensorId_t    SensorId_0;
static sensorEvent_t EventFunc_0 = NULL;
static uint32_t      EventMask_0 = 0U;

// Block memory
#if (SENSOR0_BLOCK_NUM != 0U) && (SENSOR0_BLOCK_SIZE != 0U)
static uint8_t       BlockMem_0[SENSOR0_BLOCK_NUM][SENSOR0_BLOCK_SIZE];
static uint32_t      BlockCnt_0 = 0U;
#endif

// VSI interrupt handler
void ARM_VSI0_Handler (void);
void ARM_VSI0_Handler (void) {
  uint32_t event;

  event = ARM_VSI0->IRQ.Status;
  ARM_VSI0->IRQ.Clear = event;
  __DSB();
  __ISB();

  if ((EventFunc_0 != NULL) && ((event & EventMask_0) != 0U)) {
    EventFunc_0(SensorId_0, event);
  }
}

// Initialize VSI
int32_t VSI0_Initialize (void);
int32_t VSI0_Initialize (void) {
  uint32_t n;
  char *p;

  // Register sensor name
  n = strlen(SENSOR0_NAME);
  ARM_VSI0->SENSOR_NAME_LEN = n;
  for (p = SENSOR0_NAME ; n!= 0U; n--) {
    ARM_VSI0->SENSOR_NAME_CHAR = *p++;
  }
  if (ARM_VSI0->SENSOR_NAME_VALID == 0U) {
    return SENSOR_ERROR;
  }

  // Initialize VSI peripheral
  ARM_VSI0->Timer.Control = 0U;
  ARM_VSI0->DMA.Control   = 0U;
  ARM_VSI0->IRQ.Clear     = SENSOR_EVENT_DATA | SENSOR_EVENT_OVERFLOW;
  ARM_VSI0->IRQ.Enable    = SENSOR_EVENT_DATA | SENSOR_EVENT_OVERFLOW;
  ARM_VSI0->CONTROL       = 0U;

  // Enable peripheral interrupts
//NVIC_EnableIRQ(ARM_VSI0_IRQn);
  NVIC->ISER[(((uint32_t)ARM_VSI0_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)ARM_VSI0_IRQn) & 0x1FUL));
  __DSB();
  __ISB();

  return SENSOR_OK;
}

// Uninitialize VSI
int32_t VSI0_Uninitialize (void);
int32_t VSI0_Uninitialize (void) {

  // Disable peripheral interrupts
//NVIC_DisableIRQ(ARM_VSI0_IRQn);
  NVIC->ICER[(((uint32_t)ARM_VSI0_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)ARM_VSI0_IRQn) & 0x1FUL));
  __DSB();
  __ISB();

  // Uninitialize VSI peripheral
  ARM_VSI0->Timer.Control = 0U;
  ARM_VSI0->DMA.Control   = 0U;
  ARM_VSI0->IRQ.Clear     = SENSOR_EVENT_DATA | SENSOR_EVENT_OVERFLOW;
  ARM_VSI0->IRQ.Enable    = 0U;
  ARM_VSI0->CONTROL       = 0U;

  return SENSOR_OK;
}

// Register sensor events
static int32_t RegisterEvents_0 (sensorId_t id, sensorEvent_t event_cb, uint32_t event_mask) {

  SensorId_0  = id;
  EventFunc_0 = event_cb;
  EventMask_0 = event_mask;

  return SENSOR_OK;
}

// Enable sensor
static int32_t Enable_0 (void) {
  int32_t ret = SENSOR_ERROR;

  #if   (SENSOR0_FIFO_SIZE != 0U)
    ARM_VSI0->SAMPLE_SIZE    = SENSOR0_SAMPLE_SIZE;
    ARM_VSI0->DATA_THRESHOLD = SENSOR0_DATA_THRESHOLD;
    ARM_VSI0->FIFO_SIZE      = SENSOR0_FIFO_SIZE;
    ARM_VSI0->CONTROL        = CONTROL_ENABLE_Msk;
    ARM_VSI0->Timer.Interval = SENSOR0_SAMPLE_INTERVAL;
    ARM_VSI0->Timer.Control  = ARM_VSI_Timer_Periodic_Msk |
                             #if (SENSOR0_DATA_THRESHOLD == 1U)
                               ARM_VSI_Timer_Trig_IRQ_Msk |
                             #endif
                               ARM_VSI_Timer_Run_Msk;
    ret = SENSOR_OK;
  #elif (SENSOR0_BLOCK_NUM != 0U) && (SENSOR0_BLOCK_SIZE != 0U)
    ARM_VSI0->CONTROL        = CONTROL_ENABLE_Msk |
                               CONTROL_DMA_Msk;
    BlockCnt_0               = 0U;
    ARM_VSI0->DMA.Address    = (uint32_t)BlockMem_0;
    ARM_VSI0->DMA.BlockNum   = SENSOR0_BLOCK_NUM;
    ARM_VSI0->DMA.BlockSize  = SENSOR0_BLOCK_SIZE;
    ARM_VSI0->DMA.Control    = ARM_VSI_DMA_Direction_P2M |
                               ARM_VSI_DMA_Enable_Msk;
    ARM_VSI0->Timer.Interval = SENSOR0_SAMPLE_INTERVAL *
                              (SENSOR0_BLOCK_SIZE / SENSOR0_SAMPLE_SIZE);
    ARM_VSI0->Timer.Control  = ARM_VSI_Timer_Periodic_Msk |
                               ARM_VSI_Timer_Trig_DMA_Msk |
                               ARM_VSI_Timer_Trig_IRQ_Msk |
                               ARM_VSI_Timer_Run_Msk;
    ret = SENSOR_OK;
  #endif

  return ret;
}

// Disable sensor
static int32_t Disable_0 (void) {

  #if   (SENSOR0_FIFO_SIZE != 0U)
    ARM_VSI0->Timer.Control  = 0U;
    ARM_VSI0->CONTROL        = 0U;
  #elif (SENSOR0_BLOCK_NUM != 0U) && (SENSOR0_BLOCK_SIZE != 0U)
    ARM_VSI0->Timer.Control  = 0U;
    ARM_VSI0->DMA.Control    = 0U;
    ARM_VSI0->CONTROL        = 0U;
  #endif

  return SENSOR_OK;
}

// Get overflow status
static uint32_t GetOverflow_0 (void) {
  return (ARM_VSI0->STATUS & STATUS_OVERFLOW_Msk);
}

// Read samples from sensor
#if (SENSOR0_FIFO_SIZE != 0U)
static uint32_t ReadSamples (uint32_t num_samples, void *buf) {
  uint32_t num;
  uint32_t n, m;
  uint8_t *p;

  num = ARM_VSI0->SAMPLE_COUNT;
  if (num > num_samples) {
    num = num_samples;
  }

  p = (uint8_t *)buf;
  for (n = num; n != 0U; n--) {
    for (m = SENSOR0_SAMPLE_SIZE; m != 0U; m--) {
      *p++ = (uint8_t)ARM_VSI0->SAMPLE_PORT;
    }
  }
  
  return num;
}
#endif

// Get block data
#if (SENSOR0_BLOCK_NUM != 0U) && (SENSOR0_BLOCK_SIZE != 0U)
static void * GetBlockData_0 (void) {
  void *p = NULL;

  if (ARM_VSI0->Timer.Count > BlockCnt_0) {
    p = &BlockMem_0[BlockCnt_0 & (SENSOR0_BLOCK_SIZE - 1U)][0];
    BlockCnt_0++;
  }

  return p;
}
#endif

// Exported sensor functions
sensorDrvHW_t sensorDrvHW_0 = {
  RegisterEvents_0,
  Enable_0,
  Disable_0,
  GetOverflow_0,
#if (SENSOR0_FIFO_SIZE != 0U)
  ReadSamples,
#else
  NULL,
#endif
#if (SENSOR0_BLOCK_NUM != 0U) && (SENSOR0_BLOCK_SIZE != 0U)
  GetBlockData_0
#else
  NULL
#endif
};

#endif
