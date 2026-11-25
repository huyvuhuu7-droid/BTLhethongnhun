/* gfx.h - vẽ vào framebuffer 128x64 1-bit (định dạng SSD1306: page-major, bit0 = hàng trên).
 * File này KHÔNG phụ thuộc phần cứng -> chạy được trên PC. */
#ifndef GFX_H
#define GFX_H
#include <stdint.h>
#include "app_config.h"

uint8_t *Gfx_Buffer(void);                         /* SCREEN_W*SCREEN_H/8 byte, đưa cho SSD1306_Flush */
void Gfx_Clear(void);
void Gfx_Pixel(int x, int y, uint8_t color);       /* color: 1 = sáng, 0 = tối */
void Gfx_FillRect(int x, int y, int w, int h, uint8_t color);
void Gfx_DrawRect(int x, int y, int w, int h, uint8_t color);   /* chỉ viền */
void Gfx_HLine(int x, int y, int w, uint8_t color);

/* Bitmap: mỗi hàng ceil(w/8) byte, bit cao = pixel trái (xuất từ image2cpp "horizontal").
 * Chỉ pixel bit 1 được vẽ (pixel 0 = trong suốt). */
void Gfx_Bitmap(int x, int y, const uint8_t *bmp, int w, int h, uint8_t color);

/* Font 3x5 (chữ hoa, số, ':' '-'), scale = hệ số phóng to. */
void Gfx_Text(int x, int y, const char *s, int scale, uint8_t color);
int  Gfx_TextWidth(const char *s, int scale);
void Gfx_TextCentered(int y, const char *s, int scale, uint8_t color);

#endif
