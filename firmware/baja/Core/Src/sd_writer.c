#include "main.h"
#include "fatfs.h"
#include "diagnostics.h"

#define LOG_INDEX_MAX 9999U

static bool SelectUnusedFilename(char *filename, size_t filename_size,
                                 uint16_t *file_index)
{
    FILINFO info;

    while (*file_index <= LOG_INDEX_MAX) {
        FRESULT result;

        snprintf(filename, filename_size, "RUN_%04u.CSV", *file_index);
        result = f_stat(filename, &info);
        if (result == FR_NO_FILE) {
            return true;
        }
        if (result != FR_OK) {
            return false;
        }
        (*file_index)++;
    }
    return false;
}

static bool CloseLog(FIL *file)
{
    bool success = true;

    if (f_sync(file) != FR_OK) {
        g_diagnostics.sd_sync_errors++;
        success = false;
    }
    if (f_close(file) != FR_OK) {
        g_diagnostics.sd_write_errors++;
        success = false;
    }
    return success;
}

void Task_SDWriter(void const * argument)
{
    FATFS fs;
    FIL file;
    data_record_t record;
    char csv_line[192];
    char filename[20];
    uint16_t file_index = 1U;
    uint32_t file_open_time = 0U;
    uint32_t engine_below_since = 0U;
    uint32_t last_sync_time = 0U;
    uint32_t records_since_sync = 0U;
    bool mounted = false;
    bool is_logging = false;

    (void)argument;

    for (;;) {
        uint32_t now_ms;

        if (!mounted) {
            if (f_mount(&fs, "", 1) != FR_OK ||
                !SelectUnusedFilename(filename, sizeof(filename), &file_index)) {
                g_diagnostics.sd_mount_errors++;
                Diagnostics_Heartbeat(DIAG_HEARTBEAT_SD);
                osDelay(500);
                continue;
            }
            mounted = true;
        }

        if (xQueueReceive(csvDataQueue, &record, pdMS_TO_TICKS(250)) != pdPASS) {
            Diagnostics_Heartbeat(DIAG_HEARTBEAT_SD);
            continue;
        }

        now_ms = HAL_GetTick();
        if (!is_logging && record.engine_rpm >= ENGINE_START_RPM) {
            if (!SelectUnusedFilename(filename, sizeof(filename), &file_index) ||
                f_open(&file, filename, FA_WRITE | FA_CREATE_NEW) != FR_OK) {
                g_diagnostics.sd_write_errors++;
                f_mount(NULL, "", 0);
                mounted = false;
                Diagnostics_Heartbeat(DIAG_HEARTBEAT_SD);
                continue;
            }
            if (f_puts("Time_ms,Eng_RPM,Sec_RPM,Speed_KMH,Belt_C,Roll_deg,"
                       "Pitch_deg,Z_mg,Susp_FR,Susp_RR,Sensor_Status\r\n",
                       &file) < 0) {
                g_diagnostics.sd_write_errors++;
                f_close(&file);
                f_mount(NULL, "", 0);
                mounted = false;
                Diagnostics_Heartbeat(DIAG_HEARTBEAT_SD);
                continue;
            }
            is_logging = true;
            file_open_time = now_ms;
            last_sync_time = now_ms;
            records_since_sync = 0U;
            engine_below_since = 0U;
        }

        if (is_logging) {
            int length = snprintf(
                csv_line, sizeof(csv_line),
                "%lu,%.1f,%.1f,%.1f,%.1f,%d,%d,%d,%ld,%ld,%u\r\n",
                (unsigned long)record.timestamp_ms,
                record.engine_rpm, record.secondary_rpm, record.ground_speed_kmh,
                record.belt_temp_c, record.imu_roll_deg, record.imu_pitch_deg,
                record.imu_z_mg, (long)record.susp_front_counts,
                (long)record.susp_rear_counts, record.sensor_status);

            if (length <= 0 || (size_t)length >= sizeof(csv_line) ||
                f_puts(csv_line, &file) < 0) {
                g_diagnostics.sd_write_errors++;
                f_close(&file);
                f_mount(NULL, "", 0);
                mounted = false;
                is_logging = false;
                Diagnostics_Heartbeat(DIAG_HEARTBEAT_SD);
                continue;
            }
            records_since_sync++;

            if (record.engine_rpm <= ENGINE_STOP_RPM) {
                if (engine_below_since == 0U) {
                    engine_below_since = now_ms;
                } else if ((now_ms - engine_below_since) >= ENGINE_STOP_DELAY_MS) {
                    CloseLog(&file);
                    is_logging = false;
                    file_index++;
                    engine_below_since = 0U;
                }
            } else {
                engine_below_since = 0U;
            }

            if (is_logging && (now_ms - file_open_time) >= FILE_ROTATION_MS) {
                CloseLog(&file);
                is_logging = false;
                file_index++;
            }

            if (is_logging &&
                (records_since_sync >= SD_SYNC_RECORDS ||
                 (now_ms - last_sync_time) >= SD_SYNC_INTERVAL_MS)) {
                if (f_sync(&file) != FR_OK) {
                    g_diagnostics.sd_sync_errors++;
                    f_close(&file);
                    f_mount(NULL, "", 0);
                    mounted = false;
                    is_logging = false;
                } else {
                    records_since_sync = 0U;
                    last_sync_time = now_ms;
                }
            }
        }

        Diagnostics_Heartbeat(DIAG_HEARTBEAT_SD);
    }
}
