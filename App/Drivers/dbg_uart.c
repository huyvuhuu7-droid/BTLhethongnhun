#include "dbg_uart.h"

#if DEBUG_UART

#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_i2c.h"
#include "adxl345.h"
#include "ssd1306.h"

/* Chọn driver UART (tự động):
 *  - CubeMX đã bật USART1 (sinh usart.c/huart1, stm32f1xx_hal_conf.h có HAL_UART_MODULE_ENABLED) -> dùng HAL.
 *  - Chưa bật -> dùng driver thanh ghi dự phòng bên dưới (PA9 = USART1_TX, baud = DEBUG_UART_BAUD). */
#if defined(HAL_UART_MODULE_ENABLED)
  #define DBG_USE_HAL   1
  #include "usart.h"                                  /* extern UART_HandleTypeDef huart1; */
#else
  #define DBG_USE_HAL   0
#endif

#if DBG_USE_HAL
/* ------------------------------------------------------------------ UART do CubeMX sinh (HAL)
 * MX_USART1_UART_Init() đã được main.c gọi trước MX_FREERTOS_Init() nên ở đây không cần khởi tạo gì.
 * Chỉ Dbg_Task (hoặc App_Init trước khi scheduler chạy) được in -> không có 2 nơi gọi HAL_UART_Transmit cùng lúc. */
static void tx_bytes(const char *p, size_t n)
{
    if (n) (void)HAL_UART_Transmit(&huart1, (uint8_t *)p, (uint16_t)n, (uint32_t)(10u + n));
}

void Dbg_Init(void) { }

uint32_t    Dbg_Baud(void)       { return huart1.Init.BaudRate; }
const char *Dbg_DriverName(void) { return "HAL (CubeMX USART1)"; }

#else
/* ------------------------------------------------------------------ USART1 TX bằng thanh ghi (dự phòng)
 * PA9 = USART1_TX (không remap). Chỉ bật bộ phát; PA10 (RX) không dùng. */
static void tx_bytes(const char *p, size_t n)
{
    while (n--) {
        uint32_t guard = 200000u;                     /* tránh treo nếu USART lỗi; 1 byte @115200 ≈ 87 µs */
        while (!(USART1->SR & USART_SR_TXE)) {
            if (--guard == 0u) return;
        }
        USART1->DR = (uint8_t)*p++;
    }
}

void Dbg_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Pin   = GPIO_PIN_9;
    g.Mode  = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &g);

    USART1->CR1 = 0;
    USART1->CR2 = 0;
    USART1->CR3 = 0;
    USART1->BRR = (HAL_RCC_GetPCLK2Freq() + (DEBUG_UART_BAUD / 2u)) / DEBUG_UART_BAUD;   /* 8N1, oversampling x16 */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE;
}

uint32_t    Dbg_Baud(void)       { return DEBUG_UART_BAUD; }
const char *Dbg_DriverName(void) { return "register (khong dung CubeMX)"; }
#endif /* DBG_USE_HAL */

/* ------------------------------------------------------------------ ghi trực tiếp */
void Dbg_WriteStr(const char *s)
{
    if (s) tx_bytes(s, strlen(s));
}

void Dbg_NewLine(void)
{
    tx_bytes("\r\n", 2);
}

static void put_uint_pad(uint32_t v, int width)       /* in số không dấu, đệm khoảng trắng bên trái cho đủ 'width' (<= 12) */
{
    char tmp[11], buf[24];
    int  d = 0, n = 0;
    do { tmp[d++] = (char)('0' + (v % 10u)); v /= 10u; } while (v);
    for (int i = d; i < width && n < 12; i++) buf[n++] = ' ';
    while (d) buf[n++] = tmp[--d];
    tx_bytes(buf, (size_t)n);
}

void Dbg_WriteInt(int32_t v)
{
    if (v < 0) { tx_bytes("-", 1); put_uint_pad((uint32_t)(-(int64_t)v), 0); }
    else       { put_uint_pad((uint32_t)v, 0); }
}

void Dbg_WriteHex8(uint8_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    char buf[4] = { '0', 'x', hex[v >> 4], hex[v & 0x0Fu] };
    tx_bytes(buf, 4);
}


#endif /* DEBUG_UART */
