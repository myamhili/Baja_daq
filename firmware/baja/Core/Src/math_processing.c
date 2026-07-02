#include "main.h"

float Calculate_RPM(uint32_t delta_ticks, float ppr) {
    uint32_t pclk1;
    uint32_t tim_clock_hz;

    if (delta_ticks == 0) return 0.0f; 

    pclk1 = HAL_RCC_GetPCLK1Freq();
    tim_clock_hz = ((RCC->CFGR & RCC_CFGR_PPRE1) == RCC_HCLK_DIV1) ? pclk1 : (pclk1 * 2U);

    return (60.0f * (float)tim_clock_hz) / ((float)delta_ticks * ppr);
}

float Calculate_Ground_Speed_KMH(float secondary_rpm) {
    float wheel_rpm = secondary_rpm / GEARBOX_RATIO;
    return (wheel_rpm * 3.14159f * TIRE_DIAMETER_M * 60.0f) / 1000.0f;
}

void Task_MathProcessing(void const * argument) {
    (void)argument;

    for(;;) {
        osDelay(100);
    }
}

__weak float Read_MLX90614_Temp(void) {
    return 0.0f;
}

__weak void Read_ISM330DHCX_IMU(int16_t *roll, int16_t *pitch, int16_t *z_gforce) {
    if (roll != NULL) {
        *roll = 0;
    }
    if (pitch != NULL) {
        *pitch = 0;
    }
    if (z_gforce != NULL) {
        *z_gforce = 0;
    }
}
