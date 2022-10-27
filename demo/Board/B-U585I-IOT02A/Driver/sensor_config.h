/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 */

#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

// Sensor Configuration

// Temperature
#define SENSOR0_NAME            "Temperature"
#define SENSOR0_SAMPLE_SIZE     4U
#define SENSOR0_SAMPLE_INTERVAL 1000000U
#define SENSOR0_DATA_THRESHOLD  0U
#define SENSOR0_FIFO_SIZE       4U
#define SENSOR0_BLOCK_SIZE      0U
#define SENSOR0_BLOCK_NUM       0U

// Humidity
#define SENSOR1_NAME            "Humidity"
#define SENSOR1_SAMPLE_SIZE     4U
#define SENSOR1_SAMPLE_INTERVAL 1000000U
#define SENSOR1_DATA_THRESHOLD  0U
#define SENSOR1_FIFO_SIZE       4U
#define SENSOR1_BLOCK_SIZE      0U
#define SENSOR1_BLOCK_NUM       0U

// Pressure
#define SENSOR2_NAME            "Pressure"
#define SENSOR2_SAMPLE_SIZE     4U
#define SENSOR2_SAMPLE_INTERVAL 25000U
#define SENSOR2_DATA_THRESHOLD  0U
#define SENSOR2_FIFO_SIZE       4U
#define SENSOR2_BLOCK_SIZE      0U
#define SENSOR2_BLOCK_NUM       0U

// Accelerometer
#define SENSOR3_NAME            "Accelerometer"
#define SENSOR3_SAMPLE_SIZE     6U
#define SENSOR3_SAMPLE_INTERVAL 9615U
#define SENSOR3_DATA_THRESHOLD  0U
#define SENSOR3_FIFO_SIZE       6U
#define SENSOR3_BLOCK_SIZE      0U
#define SENSOR3_BLOCK_NUM       0u

// Gyroscope
#define SENSOR4_NAME            "Gyroscope"
#define SENSOR4_SAMPLE_SIZE     6U
#define SENSOR4_SAMPLE_INTERVAL 9615U
#define SENSOR4_DATA_THRESHOLD  0U
#define SENSOR4_FIFO_SIZE       6U
#define SENSOR4_BLOCK_SIZE      0U
#define SENSOR4_BLOCK_NUM       0U

// Magnetometer
#define SENSOR5_NAME            "Magnetometer"
#define SENSOR5_SAMPLE_SIZE     6U
#define SENSOR5_SAMPLE_INTERVAL 10000U
#define SENSOR5_DATA_THRESHOLD  0U
#define SENSOR5_FIFO_SIZE       6U
#define SENSOR5_BLOCK_SIZE      0U
#define SENSOR5_BLOCK_NUM       0U

#endif /* SENSOR_CONFIG_H */
