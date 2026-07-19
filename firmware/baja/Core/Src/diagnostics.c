#include "diagnostics.h"
#include "task.h"

diagnostics_t g_diagnostics;
static volatile uint32_t task_heartbeats;

void Diagnostics_Heartbeat(uint32_t task_bit)
{
  taskENTER_CRITICAL();
  task_heartbeats |= task_bit;
  taskEXIT_CRITICAL();
}

uint32_t Diagnostics_TakeHeartbeats(void)
{
  uint32_t heartbeats;

  taskENTER_CRITICAL();
  heartbeats = task_heartbeats;
  task_heartbeats = 0U;
  taskEXIT_CRITICAL();

  return heartbeats;
}
