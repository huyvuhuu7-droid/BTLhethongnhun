#ifndef APP_RTOS_H
#define APP_RTOS_H
/* Gọi 1 lần trong main.c, sau các MX_xxx_Init() và TRƯỚC khi scheduler chạy. */
void App_Init(void);
#endif
