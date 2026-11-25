#ifndef ADXL345_H
#define ADXL345_H
#include <stdbool.h>

#define ADXL345_ADDR7   0x53          /* chân ALT ADDRESS nối GND; nối VCC thì là 0x1D */

bool ADXL345_Init(void);              /* kiểm tra DEVID = 0xE5, bật đo ±2g full-res 100 Hz */
bool ADXL345_ReadG(float *gx, float *gy, float *gz);   /* đơn vị g */

#endif
