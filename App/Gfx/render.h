#ifndef RENDER_H
#define RENDER_H
#include <stdint.h>
#include "game_types.h"

void Render_Init(void);
/* Vẽ 1 khung hình vào framebuffer (Gfx_Buffer). Chỉ ĐỌC GameData, không sửa.
 * now_ms: thời gian hệ thống, dùng cho hiệu ứng nhấp nháy chữ. */
void Render_Frame(const GameData_t *g, uint32_t now_ms);
#endif
