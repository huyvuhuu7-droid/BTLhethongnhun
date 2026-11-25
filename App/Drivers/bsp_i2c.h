/* bsp_i2c.h - truy cập I2C có mutex. ADXL345 và OLED dùng chung 1 bus I2C nên mọi
 * giao dịch PHẢI đi qua đây, tuyệt đối không gọi HAL_I2C_* trực tiếp ở nơi khác. */
#ifndef BSP_I2C_H
#define BSP_I2C_H
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

void BspI2c_Init(I2C_HandleTypeDef *hi2c);          /* gọi 1 lần, TRƯỚC khi scheduler chạy */
bool BspI2c_Write(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len);
bool BspI2c_Read (uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len);
bool BspI2c_Probe(uint8_t addr7);                   /* true nếu có thiết bị ACK ở địa chỉ này (không ghi gì vào thiết bị) */

#endif
