#include "main.h"
#include "diagnostics.h"
#include "sensors.h"

static uint8_t ClampToU8(float value)
{
    if (value <= 0.0f) {
        return 0U;
    }
    if (value >= 254.0f) {
        return 254U;
    }
    return (uint8_t)value;
}

static uint8_t CounterToU8(uint32_t value)
{
    return value > 255U ? 255U : (uint8_t)value;
}

static void SendDiagnosticsCAN(void)
{
    CAN_TxHeaderTypeDef header = TxHeader;
    uint8_t data[8];
    uint32_t mailbox;

    header.StdId = 0x101U;
    data[0] = CounterToU8(g_diagnostics.queue_drops);
    data[1] = CounterToU8(g_diagnostics.sd_mount_errors);
    data[2] = CounterToU8(g_diagnostics.sd_write_errors);
    data[3] = CounterToU8(g_diagnostics.sd_sync_errors);
    data[4] = CounterToU8(g_diagnostics.mlx_errors);
    data[5] = CounterToU8(g_diagnostics.imu_errors);
    data[6] = CounterToU8(g_diagnostics.can_tx_drops);
    data[7] = CounterToU8(g_diagnostics.can_bus_offs);

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U ||
        HAL_CAN_AddTxMessage(&hcan1, &header, data, &mailbox) != HAL_OK) {
        g_diagnostics.can_tx_drops++;
    }
}

void Task_DataGathering(void const * argument)
{
    data_record_t record = {0};
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t last_sensor_retry_ms = 0U;
    uint32_t sample_count = 0U;
    bool mlx_ready;
    bool imu_ready;
    bool temp_valid = false;

    (void)argument;

    mlx_ready = MLX90614_Init();
    imu_ready = ISM330DHCX_Init();
    if (!mlx_ready) {
        g_diagnostics.mlx_errors++;
    }
    if (!imu_ready) {
        g_diagnostics.imu_errors++;
    }

    for (;;) {
        uint32_t now_ms = HAL_GetTick();

        record.timestamp_ms = now_ms;
        record.engine_rpm = Calculate_RPM_With_Timeout(
            engine_delta_ticks, ENGINE_PPR, engine_last_pulse_ms, now_ms);
        record.secondary_rpm = Calculate_RPM_With_Timeout(
            secondary_delta_ticks, SECONDARY_PPR, secondary_last_pulse_ms, now_ms);
        record.ground_speed_kmh = Calculate_Ground_Speed_KMH(record.secondary_rpm);
        record.sensor_status = 0U;

        /* The MLX90614 updates much slower than the 100 Hz acquisition loop. */
        if ((sample_count % 10U) == 0U) {
            if (mlx_ready) {
                temp_valid = MLX90614_ReadObjectTemp(&record.belt_temp_c);
                if (!temp_valid) {
                    mlx_ready = false;
                    g_diagnostics.mlx_errors++;
                }
            } else {
                temp_valid = false;
            }
        }
        if (temp_valid) {
            record.sensor_status |= SENSOR_STATUS_TEMP_VALID;
        }

        if (imu_ready && ISM330DHCX_ReadOrientation(
                &record.imu_roll_deg, &record.imu_pitch_deg, &record.imu_z_mg)) {
            record.sensor_status |= SENSOR_STATUS_IMU_VALID;
        } else {
            record.imu_roll_deg = 0;
            record.imu_pitch_deg = 0;
            record.imu_z_mg = 0;
            if (imu_ready) {
                g_diagnostics.imu_errors++;
            }
            imu_ready = false;
        }

        if ((!mlx_ready || !imu_ready) &&
            (now_ms - last_sensor_retry_ms) >= 1000U) {
            if (!mlx_ready) {
                mlx_ready = MLX90614_Init();
            }
            if (!imu_ready) {
                imu_ready = ISM330DHCX_Init();
            }
            last_sensor_retry_ms = now_ms;
        }

        record.susp_front_counts = (int32_t)adc_buffer[0] -
                                   (int32_t)susp_zero_front_rt;
        record.susp_rear_counts = (int32_t)adc_buffer[1] -
                                  (int32_t)susp_zero_rear_rt;

        if (xQueueSend(csvDataQueue, &record, pdMS_TO_TICKS(2)) != pdPASS) {
            g_diagnostics.queue_drops++;
        }

        TxData[0] = (uint8_t)((uint16_t)record.engine_rpm >> 8U);
        TxData[1] = (uint8_t)((uint16_t)record.engine_rpm & 0xFFU);
        TxData[2] = ClampToU8(record.ground_speed_kmh);
        TxData[3] = (record.sensor_status & SENSOR_STATUS_TEMP_VALID) != 0U
                    ? ClampToU8(record.belt_temp_c) : 0xFFU;
        TxData[4] = 0U;
        if ((record.sensor_status & SENSOR_STATUS_TEMP_VALID) != 0U &&
            record.belt_temp_c > BELT_TEMP_LIMIT_C) {
            TxData[4] |= 0x01U;
        }
        if (record.engine_rpm > 3800.0f) {
            TxData[4] |= 0x02U;
        }
        if ((record.sensor_status & SENSOR_STATUS_TEMP_VALID) == 0U) {
            TxData[4] |= 0x04U;
        }
        if ((record.sensor_status & SENSOR_STATUS_IMU_VALID) == 0U) {
            TxData[4] |= 0x08U;
        }
        TxData[5] = 0U;
        TxData[6] = 0U;
        TxData[7] = 0U;

        if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U ||
            HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK) {
            g_diagnostics.can_tx_drops++;
        }
        if ((sample_count % 100U) == 0U) {
            SendDiagnosticsCAN();
        }

        Diagnostics_Heartbeat(DIAG_HEARTBEAT_DATA);
        sample_count++;
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(10));
    }
}
