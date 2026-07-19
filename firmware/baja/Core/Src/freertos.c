/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "main.h"
#include "cmsis_os.h"
#include "monitoring.h"

osThreadId dataGatheringTaskHandle;
osThreadId sdWriterTaskHandle;
osThreadId stackWatermarkTaskHandle;

void Task_DataGathering(void const * argument);
void Task_SDWriter(void const * argument);
void Task_StackWatermark(void const * argument);

void MX_FREERTOS_Init(void);

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);
void vApplicationMallocFailedHook(void);

__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  (void)xTask;
  (void)pcTaskName;
  __disable_irq();
  for(;;)
  {
  }
}

__weak void vApplicationMallocFailedHook(void)
{
  __disable_irq();
  for(;;)
  {
  }
}

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void MX_FREERTOS_Init(void)
{
  csvDataQueue = xQueueCreate(CSV_QUEUE_DEPTH, sizeof(data_record_t));
  if (csvDataQueue == NULL) {
    Error_Handler();
  }

  osThreadDef(dataGatheringTask, Task_DataGathering, osPriorityHigh, 0, 256);
  dataGatheringTaskHandle = osThreadCreate(osThread(dataGatheringTask), NULL);
  if (dataGatheringTaskHandle == NULL) {
    Error_Handler();
  }

  osThreadDef(sdWriterTask, Task_SDWriter, osPriorityBelowNormal, 0, 1024);
  sdWriterTaskHandle = osThreadCreate(osThread(sdWriterTask), NULL);
  if (sdWriterTaskHandle == NULL) {
    Error_Handler();
  }

  osThreadDef(stackWatermarkTask, Task_StackWatermark, osPriorityNormal, 0, 256);
  stackWatermarkTaskHandle = osThreadCreate(osThread(stackWatermarkTask), NULL);
  if (stackWatermarkTaskHandle == NULL) {
    Error_Handler();
  }
}
