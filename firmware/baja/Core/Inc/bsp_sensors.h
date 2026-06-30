/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SENSORS_H
#define __BSP_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */

/**
  * @brief  Initializes the sensor board peripherals.
  * @retval None
  */
void BSP_SENSORS_Init(void);

/**
  * @brief  Reads data from the sensor.
  * @param  data: pointer to the buffer to store the data.
  * @retval None
  */
void BSP_SENSORS_ReadData(uint8_t *data);


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__BSP_SENSORS_H */

