/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
// --- Vehicle & Hardware Constants ---
#define ENGINE_PPR            2.0f         // Pulses Per Revolution
#define SECONDARY_PPR         4.0f         
#define GEARBOX_RATIO         8.5f         
#define TIRE_DIAMETER_M       0.5842f      
#define BELT_TEMP_LIMIT_C     88.0f        
#define ENGINE_START_RPM      800.0f       
#define ENGINE_STOP_RPM       500.0f
#define ENGINE_STOP_DELAY_MS  1000U
#define RPM_TIMEOUT_MS        500U
#define FILE_ROTATION_MS      900000
#define CSV_QUEUE_DEPTH       32U
#define SD_SYNC_INTERVAL_MS   1000U
#define SD_SYNC_RECORDS       100U
#define SENSOR_STATUS_TEMP_VALID  (1U << 0)
#define SENSOR_STATUS_IMU_VALID   (1U << 1)

typedef struct {
  uint32_t timestamp_ms;
  float engine_rpm;
  float secondary_rpm;
  float ground_speed_kmh;
  float belt_temp_c;
  int16_t imu_roll_deg;
  int16_t imu_pitch_deg;
  int16_t imu_z_mg;
  int32_t susp_front_counts;
  int32_t susp_rear_counts;
  uint16_t sensor_status;
} data_record_t;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Init_CAN_Broadcast(void);
void Calibrate_Suspension_Zeros(void);
void Start_Timing_Inputs(void);
float Calculate_RPM(uint32_t delta_ticks, float ppr);
float Calculate_RPM_With_Timeout(uint32_t delta_ticks, float ppr,
                                 uint32_t last_pulse_ms, uint32_t now_ms);
float Calculate_Ground_Speed_KMH(float secondary_rpm);

extern ADC_HandleTypeDef hadc1;
extern CAN_HandleTypeDef hcan1;
extern IWDG_HandleTypeDef hiwdg;
extern SD_HandleTypeDef hsd;
extern TIM_HandleTypeDef htim5;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_sdio_rx;
extern DMA_HandleTypeDef hdma_sdio_tx;
extern QueueHandle_t csvDataQueue;
extern volatile uint16_t adc_buffer[2];
extern uint32_t susp_zero_front_rt;
extern uint32_t susp_zero_rear_rt;
extern volatile uint32_t engine_delta_ticks;
extern volatile uint32_t secondary_delta_ticks;
extern volatile uint32_t engine_last_pulse_ms;
extern volatile uint32_t secondary_last_pulse_ms;
extern CAN_TxHeaderTypeDef TxHeader;
extern uint32_t TxMailbox;
extern uint8_t TxData[8];
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_CS_Pin GPIO_PIN_1
#define SPI_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
