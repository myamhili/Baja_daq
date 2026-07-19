#include "sensors.h"
#include "i2c.h"
#include "spi.h"
#include <math.h>

#define MLX90614_ADDRESS          (0x5AU << 1)
#define MLX90614_OBJECT_TEMP_REG  0x07U
#define MLX90614_RETRIES          3U
#define MLX90614_TIMEOUT_MS       20U

#define ISM330_WHO_AM_I_REG       0x0FU
#define ISM330_WHO_AM_I_VALUE     0x6BU
#define ISM330_CTRL1_XL_REG       0x10U
#define ISM330_CTRL2_G_REG        0x11U
#define ISM330_CTRL3_C_REG        0x12U
#define ISM330_OUTX_L_G_REG       0x22U
#define ISM330_SPI_READ           0x80U
#define ISM330_SPI_TIMEOUT_MS     20U
#define RAD_TO_DEG                57.2957795f

static uint8_t Calculate_PEC(const uint8_t *data, uint32_t length)
{
  uint8_t crc = 0U;
  uint32_t i;
  uint8_t bit;

  for (i = 0U; i < length; i++) {
    crc ^= data[i];
    for (bit = 0U; bit < 8U; bit++) {
      crc = (crc & 0x80U) ? (uint8_t)((crc << 1U) ^ 0x07U)
                          : (uint8_t)(crc << 1U);
    }
  }
  return crc;
}

bool MLX90614_Init(void)
{
  return HAL_I2C_IsDeviceReady(&hi2c1, MLX90614_ADDRESS,
                               MLX90614_RETRIES, MLX90614_TIMEOUT_MS) == HAL_OK;
}

bool MLX90614_ReadObjectTemp(float *temperature_c)
{
  uint8_t response[3];
  uint8_t pec_data[5];
  uint16_t raw;
  uint32_t attempt;

  if (temperature_c == NULL) {
    return false;
  }

  for (attempt = 0U; attempt < MLX90614_RETRIES; attempt++) {
    if (HAL_I2C_Mem_Read(&hi2c1, MLX90614_ADDRESS,
                         MLX90614_OBJECT_TEMP_REG, I2C_MEMADD_SIZE_8BIT,
                         response, sizeof(response), MLX90614_TIMEOUT_MS) != HAL_OK) {
      continue;
    }

    pec_data[0] = MLX90614_ADDRESS;
    pec_data[1] = MLX90614_OBJECT_TEMP_REG;
    pec_data[2] = (uint8_t)(MLX90614_ADDRESS | 1U);
    pec_data[3] = response[0];
    pec_data[4] = response[1];
    if (Calculate_PEC(pec_data, sizeof(pec_data)) != response[2]) {
      continue;
    }

    raw = (uint16_t)response[0] | ((uint16_t)response[1] << 8U);
    if ((raw & 0x8000U) != 0U) {
      continue;
    }

    *temperature_c = ((float)raw * 0.02f) - 273.15f;
    return true;
  }

  return false;
}

static bool ISM330_Transfer(uint8_t reg, uint8_t *data, uint16_t length, bool read)
{
  HAL_StatusTypeDef status;
  uint8_t command = read ? (uint8_t)(reg | ISM330_SPI_READ) : reg;

  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
  status = HAL_SPI_Transmit(&hspi1, &command, 1U, ISM330_SPI_TIMEOUT_MS);
  if (status == HAL_OK) {
    status = read ? HAL_SPI_Receive(&hspi1, data, length, ISM330_SPI_TIMEOUT_MS)
                  : HAL_SPI_Transmit(&hspi1, data, length, ISM330_SPI_TIMEOUT_MS);
  }
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

  return status == HAL_OK;
}

static bool ISM330_WriteRegister(uint8_t reg, uint8_t value)
{
  return ISM330_Transfer(reg, &value, 1U, false);
}

static bool ISM330_ReadRegister(uint8_t reg, uint8_t *value)
{
  return ISM330_Transfer(reg, value, 1U, true);
}

bool ISM330DHCX_Init(void)
{
  uint8_t who_am_i = 0U;

  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
  if (!ISM330_ReadRegister(ISM330_WHO_AM_I_REG, &who_am_i) ||
      who_am_i != ISM330_WHO_AM_I_VALUE) {
    return false;
  }

  /* 104 Hz, +/-4 g; 104 Hz, +/-500 dps; block update and auto-increment. */
  return ISM330_WriteRegister(ISM330_CTRL3_C_REG, 0x44U) &&
         ISM330_WriteRegister(ISM330_CTRL1_XL_REG, 0x48U) &&
         ISM330_WriteRegister(ISM330_CTRL2_G_REG, 0x44U);
}

bool ISM330DHCX_ReadOrientation(int16_t *roll_deg, int16_t *pitch_deg,
                               int16_t *z_mg)
{
  uint8_t raw_data[12];
  int16_t accel_x_raw;
  int16_t accel_y_raw;
  int16_t accel_z_raw;
  float accel_x_mg;
  float accel_y_mg;
  float accel_z_mg;
  float roll;
  float pitch;

  if (roll_deg == NULL || pitch_deg == NULL || z_mg == NULL ||
      !ISM330_Transfer(ISM330_OUTX_L_G_REG, raw_data, sizeof(raw_data), true)) {
    return false;
  }

  accel_x_raw = (int16_t)((uint16_t)raw_data[6] | ((uint16_t)raw_data[7] << 8U));
  accel_y_raw = (int16_t)((uint16_t)raw_data[8] | ((uint16_t)raw_data[9] << 8U));
  accel_z_raw = (int16_t)((uint16_t)raw_data[10] | ((uint16_t)raw_data[11] << 8U));
  accel_x_mg = (float)accel_x_raw * 0.122f;
  accel_y_mg = (float)accel_y_raw * 0.122f;
  accel_z_mg = (float)accel_z_raw * 0.122f;

  roll = atan2f(accel_y_mg, accel_z_mg) * RAD_TO_DEG;
  pitch = atan2f(-accel_x_mg,
                 sqrtf((accel_y_mg * accel_y_mg) +
                       (accel_z_mg * accel_z_mg))) * RAD_TO_DEG;

  *roll_deg = (int16_t)roll;
  *pitch_deg = (int16_t)pitch;
  *z_mg = (int16_t)accel_z_mg;
  return true;
}
