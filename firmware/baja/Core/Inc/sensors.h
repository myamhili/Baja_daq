#ifndef INC_SENSORS_H_
#define INC_SENSORS_H_

#include "main.h"

bool MLX90614_Init(void);
bool MLX90614_ReadObjectTemp(float *temperature_c);

bool ISM330DHCX_Init(void);
bool ISM330DHCX_ReadOrientation(int16_t *roll_deg, int16_t *pitch_deg,
                               int16_t *z_mg);

#endif
