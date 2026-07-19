#include "main.h"
#include "cmsis_os.h"
#include "task.h"
#include "diagnostics.h"

extern osThreadId dataGatheringTaskHandle;
extern osThreadId sdWriterTaskHandle;

void Task_StackWatermark(void const * argument)
{
    const uint32_t required_heartbeats = DIAG_HEARTBEAT_DATA | DIAG_HEARTBEAT_SD;

    (void)argument;

    for (;;) {
        UBaseType_t data_watermark;
        UBaseType_t sd_watermark;
        uint32_t heartbeats;

        osDelay(1000);
        data_watermark = uxTaskGetStackHighWaterMark(dataGatheringTaskHandle);
        sd_watermark = uxTaskGetStackHighWaterMark(sdWriterTaskHandle);

        if (g_diagnostics.stack_min_data_words == 0U ||
            data_watermark < g_diagnostics.stack_min_data_words) {
            g_diagnostics.stack_min_data_words = data_watermark;
        }
        if (g_diagnostics.stack_min_sd_words == 0U ||
            sd_watermark < g_diagnostics.stack_min_sd_words) {
            g_diagnostics.stack_min_sd_words = sd_watermark;
        }

        heartbeats = Diagnostics_TakeHeartbeats();
        if ((heartbeats & required_heartbeats) == required_heartbeats) {
            HAL_IWDG_Refresh(&hiwdg);
        }
    }
}
