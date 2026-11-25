/* app_rtos.c - tạo task, mutex; nối Input -> Game -> Render.
 * Debug qua UART (USART1 TX = PA9, 115200 8N1): xem dbg_uart.h và docs/UART_DEBUG.md. */
#include "app_rtos.h"
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "i2c.h"
#include "app_config.h"
#include "game.h"
#include "gfx.h"
#include "render.h"
#include "input.h"
#include "bsp_i2c.h"
#include "ssd1306.h"
#include "dbg_uart.h"

static GameData_t        s_game;
static SemaphoreHandle_t s_game_mutex;

#if DEBUG_UART
static const char *const k_state_msg[] = {
    [GAME_MENU]    = "STATE -> MENU",
    [GAME_PLAYING] = "STATE -> PLAYING",
    [GAME_PAUSED]  = "STATE -> PAUSED",
    [GAME_OVER]    = "STATE -> GAME_OVER",
    [GAME_WIN]     = "STATE -> WIN",
};

/* Banner lúc khởi động (in trực tiếp, trước khi scheduler chạy). */
static void boot_banner(void)
{
    uint32_t csr = RCC->CSR;

    Dbg_NewLine();
    Dbg_WriteStr("==== BTL He thong nhung: Space Shooter | STM32F103C8T6 + FreeRTOS ===="); Dbg_NewLine();
    Dbg_WriteStr("Build   : " __DATE__ " " __TIME__);                                      Dbg_NewLine();
    Dbg_WriteStr("SYSCLK  : "); Dbg_WriteInt((int32_t)SystemCoreClock); Dbg_WriteStr(" Hz");   Dbg_NewLine();
    Dbg_WriteStr("UART    : "); Dbg_WriteStr(Dbg_DriverName()); Dbg_WriteStr(", TX=PA9, ");
    Dbg_WriteInt((int32_t)Dbg_Baud()); Dbg_WriteStr(" baud"); Dbg_NewLine();
    Dbg_WriteStr("Reset   :");
    if (csr & RCC_CSR_PORRSTF)  Dbg_WriteStr(" POWER-ON");
    if (csr & RCC_CSR_PINRSTF)  Dbg_WriteStr(" NRST-PIN");
    if (csr & RCC_CSR_SFTRSTF)  Dbg_WriteStr(" SOFTWARE");
    if (csr & RCC_CSR_IWDGRSTF) Dbg_WriteStr(" IWDG");
    if (csr & RCC_CSR_WWDGRSTF) Dbg_WriteStr(" WWDG");
    if (csr & RCC_CSR_LPWRRSTF) Dbg_WriteStr(" LOW-POWER");
    Dbg_NewLine();
    RCC->CSR |= RCC_CSR_RMVF;                                                              /* xoá cờ reset cho lần sau */

    Dbg_WriteStr("Buttons : PA0=FIRE PA1=SKILL PA2=START PA3=PAUSE (active-low, pull-up)"); Dbg_NewLine();
    Dbg_WriteStr("Debounce: "); Dbg_WriteInt(BTN_DEBOUNCE_SAMPLES); Dbg_WriteStr(" mau x ");
    Dbg_WriteInt(INPUT_TICK_MS); Dbg_WriteStr(" ms = ");
    Dbg_WriteInt(BTN_DEBOUNCE_SAMPLES * INPUT_TICK_MS); Dbg_WriteStr(" ms"); Dbg_NewLine();
}

static void report(const char *what, int ok)
{
    Dbg_WriteStr("  create "); Dbg_WriteStr(what); Dbg_WriteStr(ok ? " : OK" : " : FAIL"); Dbg_NewLine();
}
#else
#define boot_banner()           ((void)0)
#define report(what, ok)        ((void)(ok))
#endif

static void Fatal(const char *why)                 /* gọi khi tạo task/mutex thất bại: in lỗi lặp lại mỗi giây */
{
    (void)why;
    for (;;) {
        Dbg_WriteStr("FATAL: "); Dbg_WriteStr(why); Dbg_NewLine();
        HAL_Delay(1000);
    }
}

static void Game_Task(void *arg)
{
    (void)arg;
    Game_Init(&s_game);
#if DEBUG_UART
    GameState_t prev = s_game.state;
    DBG_LOG(k_state_msg[prev]);
#endif
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        InputState_t in;
        Input_Get(&in);

        xSemaphoreTake(s_game_mutex, portMAX_DELAY);
        Game_Update(&s_game, &in, GAME_TICK_MS);
#if DEBUG_UART
        GameState_t now = s_game.state;
#endif
        xSemaphoreGive(s_game_mutex);

#if DEBUG_UART
        if (now != prev) { prev = now; DBG_LOG(k_state_msg[now]); }   /* log khi game đổi trạng thái (cho thấy nút có tác dụng) */
#endif
        vTaskDelayUntil(&last, pdMS_TO_TICKS(GAME_TICK_MS));
    }
}

static void Render_Task(void *arg)
{
    (void)arg;
#if DEBUG_UART
    bool oled_fail_logged = false;
#endif
    while (!SSD1306_Init()) {
#if DEBUG_UART
        if (!oled_fail_logged) { oled_fail_logged = true; DBG_LOG("OLED init FAIL (thu lai moi 200 ms)"); }
#endif
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    DBG_LOG("OLED init OK");
    Render_Init();

    TickType_t last = xTaskGetTickCount();
    for (;;) {
        xSemaphoreTake(s_game_mutex, portMAX_DELAY);
        Render_Frame(&s_game, (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS));
        xSemaphoreGive(s_game_mutex);

        SSD1306_Flush(Gfx_Buffer());       /* chậm (I2C) -> làm NGOÀI mutex */
        vTaskDelayUntil(&last, pdMS_TO_TICKS(RENDER_TICK_MS));
    }
}

void App_Init(void)
{
    Dbg_Init();
    boot_banner();

    BspI2c_Init(&hi2c1);
    s_game_mutex = xSemaphoreCreateMutex();

    int r, ok;
    r = (s_game_mutex != NULL);                                                       report("mutex game", r);       ok = r;
    r = (xTaskCreate(Input_Task,  "input",  256, NULL, 3, NULL) == pdPASS);           report("task input  (prio 3)", r); ok &= r;
    r = (xTaskCreate(Game_Task,   "game",   384, NULL, 2, NULL) == pdPASS);           report("task game   (prio 2)", r); ok &= r;
    r = (xTaskCreate(Render_Task, "render", 384, NULL, 1, NULL) == pdPASS);           report("task render (prio 1)", r); ok &= r;
#if DEBUG_UART
    r = (xTaskCreate(Dbg_Task,    "dbg",    192, NULL, 1, NULL) == pdPASS);           report("task dbg    (prio 1)", r); ok &= r;
    Dbg_WriteStr("Heap    : free="); Dbg_WriteInt((int32_t)xPortGetFreeHeapSize()); Dbg_WriteStr(" / ");
    Dbg_WriteInt((int32_t)configTOTAL_HEAP_SIZE); Dbg_WriteStr(" bytes"); Dbg_NewLine();
#endif
    if (!ok) Fatal("tao task/mutex that bai (het heap?)");
    Dbg_WriteStr("Scheduler starting..."); Dbg_NewLine();
}
