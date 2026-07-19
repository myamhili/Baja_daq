#ifndef INC_DIAGNOSTICS_H_
#define INC_DIAGNOSTICS_H_

#include "main.h"

#define DIAG_HEARTBEAT_DATA  (1U << 0)
#define DIAG_HEARTBEAT_SD    (1U << 1)

typedef struct {
  volatile uint32_t mlx_errors;
  volatile uint32_t imu_errors;
  volatile uint32_t queue_drops;
  volatile uint32_t sd_mount_errors;
  volatile uint32_t sd_write_errors;
  volatile uint32_t sd_sync_errors;
  volatile uint32_t can_tx_drops;
  volatile uint32_t can_bus_offs;
  volatile uint32_t stack_min_data_words;
  volatile uint32_t stack_min_sd_words;
} diagnostics_t;

extern diagnostics_t g_diagnostics;

void Diagnostics_Heartbeat(uint32_t task_bit);
uint32_t Diagnostics_TakeHeartbeats(void);

#endif
