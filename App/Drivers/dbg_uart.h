/* dbg_uart.h - debug qua UART (USART1, chỉ TX = PA9, 115200 8N1).
 *
 * Driver tự chọn (xem dbg_uart.c):
 *   - Đã bật USART1 trong CubeMX (có usart.c / huart1)  -> dùng HAL_UART_Transmit.
 *   - Chưa bật                                          -> dùng driver thanh ghi dự phòng.
 * Hai cách ghi log:
 *   1) Ghi trực tiếp (chặn, polling): Dbg_WriteStr/Int/Hex8/NewLine
 *        -> chỉ dùng TRƯỚC khi scheduler chạy (App_Init) hoặc bên trong Dbg_Task.
 *   2) Ghi qua hàng đợi (KHÔNG chặn): DBG_LOG / DBG_LOG1 / DBG_LOG2
 *        -> gọi được từ mọi task (kể cả Input_Task 10 ms). Dbg_Task sẽ in ra UART sau.
 *        -> chuỗi truyền vào PHẢI là hằng chuỗi (nằm trong Flash), vì chỉ lưu con trỏ.
 * Đặt DEBUG_UART = 0 trong app_config.h để tắt hết (mọi macro biến thành rỗng).
 */
#ifndef DBG_UART_H
#define DBG_UART_H
#include <stdint.h>
#include <stddef.h>
#include "app_config.h"

#if DEBUG_UART

void Dbg_Init(void);                              /* gọi 1 lần trong App_Init, trước khi scheduler chạy */
uint32_t    Dbg_Baud(void);                       /* baud đang dùng thực tế (HAL: lấy từ huart1) */
const char *Dbg_DriverName(void);                 /* "HAL (CubeMX USART1)" hoặc "register ..." */

/* --- ghi trực tiếp --- */
void Dbg_WriteStr(const char *s);
void Dbg_WriteInt(int32_t v);
void Dbg_WriteHex8(uint8_t v);                    /* in dạng 0xNN */
void Dbg_NewLine(void);                           /* "\r\n" */

/* --- ghi qua hàng đợi (không chặn) --- */
void Dbg_Post(const char *s1, int32_t v1, const char *s2, int32_t v2, uint8_t nv);
#define DBG_LOG(s)                  Dbg_Post((s), 0, NULL, 0, 0)                          /* "[t ms] s"                */
#define DBG_LOG1(s, v)              Dbg_Post((s), (int32_t)(v), NULL, 0, 1)               /* "[t ms] s<v>"             */
#define DBG_LOG2(s, v, s2, v2)      Dbg_Post((s), (int32_t)(v), (s2), (int32_t)(v2), 2)   /* "[t ms] s<v>s2<v2>"       */

/* Task in log + quét I2C lúc khởi động + in STAT định kỳ. Tạo trong App_Init (ưu tiên thấp, bằng Render_Task). */
void Dbg_Task(void *arg);

#else

#define Dbg_Init()                  ((void)0)
#define Dbg_WriteStr(s)             ((void)0)
#define Dbg_WriteInt(v)             ((void)0)
#define Dbg_WriteHex8(v)            ((void)0)
#define Dbg_NewLine()               ((void)0)
#define DBG_LOG(s)                  ((void)0)
#define DBG_LOG1(s, v)              ((void)0)
#define DBG_LOG2(s, v, s2, v2)      ((void)0)

#endif /* DEBUG_UART */
#endif /* DBG_UART_H */
