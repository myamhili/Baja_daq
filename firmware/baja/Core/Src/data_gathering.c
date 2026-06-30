void Task_DataGathering(void const * argument) {
    float engine_rpm, secondary_rpm, ground_speed, belt_temp;
    int16_t imu_roll, imu_pitch, imu_z_gforce; 
    
    for(;;) {
        // 1. Calculate Speeds
        engine_rpm = Calculate_RPM(engine_delta_ticks, ENGINE_PPR);
        secondary_rpm = Calculate_RPM(secondary_delta_ticks, SECONDARY_PPR);
        ground_speed = Calculate_Ground_Speed_KMH(secondary_rpm);
        
        // 2. Read Sensors 
        belt_temp = Read_MLX90614_Temp(); 
        
        // Assert SPI1 CS Low on PB1
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
        Read_ISM330DHCX_IMU(&imu_roll, &imu_pitch, &imu_z_gforce);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
        
        // 3. Calculate Suspension Delta
        int32_t susp_fr_delta = adc_buffer[0] - susp_zero_front_rt;
        int32_t susp_rr_delta = adc_buffer[1] - susp_zero_rear_rt;
        
        // 4. Format CSV
        snprintf(csvBuffer, sizeof(csvBuffer), 
                 "%.1f,%.1f,%.1f,%.1f,%d,%d,%d,%ld,%ld
",
                 engine_rpm, secondary_rpm, ground_speed, belt_temp,
                 imu_roll, imu_pitch, imu_z_gforce,
                 susp_fr_delta, susp_rr_delta);
                 
        // 5. Push to SD Queue
        xQueueSend(csvDataQueue, &csvBuffer, pdMS_TO_TICKS(2));
        
        // 6. Pack and Transmit CAN Frame
        uint16_t rpm_int = (uint16_t)engine_rpm;
        TxData[0] = (uint8_t)(rpm_int >> 8);     
        TxData[1] = (uint8_t)(rpm_int & 0xFF);   
        TxData[2] = (uint8_t)ground_speed;       
        TxData[3] = (uint8_t)belt_temp;          
        
        // Byte 4: System Status / Warning Flags bitmask
        TxData[4] = 0x00;
        if (belt_temp > BELT_TEMP_LIMIT_C) TxData[4] |= 0x01; 
        if (rpm_int > 3800) TxData[4] |= 0x02; 
        
        TxData[5] = 0x00; 
        TxData[6] = 0x00; 
        TxData[7] = 0x00; 

        // Safely transmit ONLY if the hardware has an empty mailbox
        if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) > 0) {
            HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox);
        }

        // 7. Strict 10ms Delay Loop
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}