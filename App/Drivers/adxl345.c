#include "adxl345.h"
#include "bsp_i2c.h"

#define REG_DEVID        0x00
#define REG_BW_RATE      0x2C
#define REG_POWER_CTL    0x2D
#define REG_DATA_FORMAT  0x31
#define REG_DATAX0       0x32

#define DEVID_EXPECTED   0xE5
#define LSB_PER_G        256.0f       /* full-res: 3.9 mg/LSB ở mọi dải */

bool ADXL345_Init(void)
{
    uint8_t id = 0, v;
    if (!BspI2c_Read(ADXL345_ADDR7, REG_DEVID, &id, 1) || id != DEVID_EXPECTED) return false;

    v = 0x0A;  if (!BspI2c_Write(ADXL345_ADDR7, REG_BW_RATE, &v, 1))     return false;  /* 100 Hz */
    v = 0x08;  if (!BspI2c_Write(ADXL345_ADDR7, REG_DATA_FORMAT, &v, 1)) return false;  /* full-res, ±2g */
    v = 0x08;  if (!BspI2c_Write(ADXL345_ADDR7, REG_POWER_CTL, &v, 1))   return false;  /* measure */
    return true;
}

bool ADXL345_ReadG(float *gx, float *gy, float *gz)
{
    uint8_t b[6];
    if (!BspI2c_Read(ADXL345_ADDR7, REG_DATAX0, b, 6)) return false;
    *gx = (int16_t)((b[1] << 8) | b[0]) / LSB_PER_G;
    *gy = (int16_t)((b[3] << 8) | b[2]) / LSB_PER_G;
    *gz = (int16_t)((b[5] << 8) | b[4]) / LSB_PER_G;
    return true;
}
