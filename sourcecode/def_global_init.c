#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>

// --- Vehicle & Hardware Constants ---
#define TIMER_FREQ_HZ         84000000.0f  // Assuming 84MHz APB timer clock
#define ENGINE_PPR            2.0f         // Pulses Per Revolution
#define SECONDARY_PPR         4.0f         
#define GEARBOX_RATIO         8.5f         
#define TIRE_DIAMETER_M       0.5842f      
#define BELT_TEMP_LIMIT_C     88.0f        
#define ENGINE_START_RPM      800.0f       
#define FILE_ROTATION_MS      900000       

// --- FreeRTOS Globals ---
QueueHandle_t csvDataQueue;
char csvBuffer[128];

// --- Hardware Peripherals (Mapped from SKiDL) ---
extern I2C_HandleTypeDef hi2c1;  // PB6(SCL), PB7(SDA)
extern SPI_HandleTypeDef hspi1;  // PA5(SCK), PA6(MISO), PA7(MOSI)
extern CAN_HandleTypeDef hcan1;  // PB8(RX), PB9(TX)
extern TIM_HandleTypeDef htim2;  // PA0(CH1 - Eng), PA1(CH2 - Sec)
extern ADC_HandleTypeDef hadc1;  // PA2(IN2 - SuspFR), PA3(IN3 - SuspRR)

uint32_t adc_buffer[2];          // DMA buffer for 2 analog channels
uint32_t susp_zero_front_rt = 0;
uint32_t susp_zero_rear_rt = 0;

volatile uint32_t engine_delta_ticks = 0;
volatile uint32_t secondary_delta_ticks = 0;

// --- CAN Globals ---
CAN_TxHeaderTypeDef TxHeader;
uint32_t TxMailbox;
uint8_t TxData[8];

// --- Initialization Functions ---
void Init_CAN_Broadcast(void) {
    CAN_FilterTypeDef canfilterconfig;
    
    // Setup a wide-open filter (required to start the bxCAN module)
    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 0;
    canfilterconfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    canfilterconfig.FilterIdHigh = 0x0000;
    canfilterconfig.FilterIdLow = 0x0000;
    canfilterconfig.FilterMaskIdHigh = 0x0000;
    canfilterconfig.FilterMaskIdLow = 0x0000;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 14; 

    HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);
    HAL_CAN_Start(&hcan1);

    // Pre-configure the Transmission Header
    TxHeader.StdId = 0x100;           
    TxHeader.ExtId = 0x00;
    TxHeader.RTR = CAN_RTR_DATA;      
    TxHeader.IDE = CAN_ID_STD;        
    TxHeader.DLC = 8;                 
    TxHeader.TransmitGlobalTime = DISABLE;
}

// Called once in main() before FreeRTOS scheduler starts
void Calibrate_Suspension_Zeros(void) {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 2);
    HAL_Delay(10); 
    susp_zero_front_rt = adc_buffer[0];
    susp_zero_rear_rt = adc_buffer[1];
}