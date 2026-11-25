#ifndef INPUT_H
#define INPUT_H
#include "input_types.h"

void Input_Task(void *arg);               /* FreeRTOS task: đọc ADXL345 + nút */
void Input_Get(InputState_t *out);        /* Game task gọi mỗi tick; tự xoá cờ 'pressed' */

#endif
