/*
 * monitoring.c
 *
 *  Created on: Jul 2, 2026
 *      Author: Gemini
 */

#include "main.h"
#include "cmsis_os.h"
#include "task.h"
#include <stdio.h>

// Assumed task handles. These must be defined in the file where the tasks are created.
extern osThreadId dataGatheringTaskHandle;
extern osThreadId mathProcessingTaskHandle;
extern osThreadId sdWriterTaskHandle;
extern osThreadId defaultTaskHandle;


void Task_StackWatermark(void const * argument)
{
    (void)argument;

    for(;;)
    {
        // Print a header for the stack watermark information
        printf("--- Stack Watermarks ---\r\n");

        // Get and print the stack watermark for each task
        UBaseType_t waterMark;

        waterMark = uxTaskGetStackHighWaterMark(dataGatheringTaskHandle);
        printf("Data Gathering Task: %lu bytes free\r\n", waterMark * sizeof(StackType_t));

        waterMark = uxTaskGetStackHighWaterMark(mathProcessingTaskHandle);
        printf("Math Processing Task: %lu bytes free\r\n", waterMark * sizeof(StackType_t));

        waterMark = uxTaskGetStackHighWaterMark(sdWriterTaskHandle);
        printf("SD Writer Task: %lu bytes free\r\n", waterMark * sizeof(StackType_t));
        
        waterMark = uxTaskGetStackHighWaterMark(defaultTaskHandle);
        printf("Default Task: %lu bytes free\r\n", waterMark * sizeof(StackType_t));

        printf("------------------------\r\n\r\n");

        HAL_IWDG_Refresh(&hiwdg);

        // Delay for 5 seconds
        osDelay(5000);
    }
}
