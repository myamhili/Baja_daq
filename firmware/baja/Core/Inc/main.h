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
#include "fatfs.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
// --- Vehicle & Hardware Constants ---
#define TIMER_FREQ_HZ         84000000.0f  // 84MHz APB timer clock
#define ENGINE_PPR            2.0f         // Pulses Per Revolution
#define SECONDARY_PPR         4.0f         
#define GEARBOX_RATIO         8.5f         
#define TIRE_DIAMETER_M       0.5842f      
#define BELT_TEMP_LIMIT_C     88.0f        
#define ENGINE_START_RPM      800.0f       
#define FILE_ROTATION_MS      900000
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Init_CAN_Broadcast(void);
void Calibrate_Suspension_Zeros(void);
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
