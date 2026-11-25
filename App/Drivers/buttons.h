#ifndef BUTTONS_H
#define BUTTONS_H
#include <stdint.h>
#include "input_types.h"

/* Gọi đều đặn mỗi INPUT_TICK_MS. Chống dội: phải đọc BTN_DEBOUNCE_SAMPLES mẫu liên tiếp (~30 ms).
 * held: bit = nút đang giữ ; edges: bit = nút vừa được nhấn xuống ở lần gọi này. */
void Buttons_Poll(uint8_t *held, uint8_t *edges);

/* Chỉ để debug (UART): thông tin của lần Buttons_Poll gần nhất.
 * released: bit = nút vừa được nhả (sau debounce).
 * glitch  : bit = nút có 1 xung đổi mức nhưng KHÔNG giữ đủ số mẫu -> bị debounce loại bỏ. */
void Buttons_DebugInfo(uint8_t *released, uint8_t *glitch);

#endif
