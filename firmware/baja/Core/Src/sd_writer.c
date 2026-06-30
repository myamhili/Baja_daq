void Task_SDWriter(void const * argument) {
    FATFS fs;
    FIL file;
    char rxBuffer[128];
    char filename[16];
    uint16_t file_index = 1;
    uint8_t is_logging = 0;
    uint32_t file_open_time = 0;
    
    // FatFs utilizes the SDIO peripheral automatically
    f_mount(&fs, "", 1);

    for(;;) {
        // Block until data is available in the queue
        if (xQueueReceive(csvDataQueue, &rxBuffer, portMAX_DELAY) == pdPASS) {
            
            float current_engine_rpm;
            sscanf(rxBuffer, "%f", &current_engine_rpm);
            
            // --- STATE MACHINE: START LOGGING ---
            if (!is_logging && current_engine_rpm > ENGINE_START_RPM) {
                snprintf(filename, sizeof(filename), "RUN_%03d.CSV", file_index);
                if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
                    f_puts("Eng_RPM,Sec_RPM,Speed_KMH,Belt_C,Roll,Pitch,Z_G,Susp_FR,Susp_RR
", &file);
                    is_logging = 1;
                    file_open_time = HAL_GetTick();
                }
            }
            
            // --- STATE MACHINE: ACTIVE LOGGING ---
            if (is_logging) {
                f_puts(rxBuffer, &file);
                
                // 15-Minute file rotation to protect data
                if ((HAL_GetTick() - file_open_time) > FILE_ROTATION_MS) {
                    f_close(&file);
                    file_index++;
                    is_logging = 0; 
                }
                
                // Engine off triggers safe file close
                if (current_engine_rpm == 0.0f) {
                    f_close(&file);
                    file_index++;
                    is_logging = 0;
                }
                
                // Sync data periodically (every 100 loops/~1 sec) 
                static int sync_counter = 0;
                if (++sync_counter >= 100) {
                    f_sync(&file);
                    sync_counter = 0;
                }
            }
        }
    }
}