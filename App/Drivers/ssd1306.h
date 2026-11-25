#ifndef SSD1306_H
#define SSD1306_H
#include <stdint.h>
#include <stdbool.h>

#define SSD1306_ADDR7   0x3C          /* một số module là 0x3D */

bool SSD1306_Init(void);
/* Đẩy framebuffer (1024 byte) ra OLED, từng page một để nhả mutex I2C giữa các page,
 * nhờ vậy Input task vẫn đọc ADXL345 được trong lúc đang vẽ. */
bool SSD1306_Flush(const uint8_t *fb);

#endif
