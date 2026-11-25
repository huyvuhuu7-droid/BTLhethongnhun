#include "input.h"
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "app_config.h"
#include "adxl345.h"
#include "buttons.h"
#include "dbg_uart.h"

static InputState_t s_shared;             /* ghi bởi Input_Task, đọc bởi Game task */
static float s_off_x, s_off_y;            /* vị trí cân bằng (hiệu chuẩn lúc khởi động) */
static float s_fx, s_fy;                  /* giá trị đã lọc */

#if DEBUG_UART
/* ------------------------------------------------------------------ log nút bấm qua UART
 * Mỗi lần nhấn/nhả (SAU debounce) in 1 dòng:
 *   [ 12345 ms] BTN_FIRE  DOWN  glitch_rejected=2
 *   [ 12480 ms] BTN_FIRE  UP    held_ms=135  glitch_rejected=0
 * glitch_rejected = số xung đổi mức đã bị debounce loại bỏ kể từ lần nhấn/nhả trước của nút đó. */
static const char *const k_down_msg[BTN_COUNT] = {
    [BTN_FIRE]  = "BTN_FIRE  DOWN  glitch_rejected=",
    [BTN_SKILL] = "BTN_SKILL DOWN  glitch_rejected=",
    [BTN_START] = "BTN_START DOWN  glitch_rejected=",
    [BTN_PAUSE] = "BTN_PAUSE DOWN  glitch_rejected=",
};
static const char *const k_up_msg[BTN_COUNT] = {
    [BTN_FIRE]  = "BTN_FIRE  UP    held_ms=",
    [BTN_SKILL] = "BTN_SKILL UP    held_ms=",
    [BTN_START] = "BTN_START UP    held_ms=",
    [BTN_PAUSE] = "BTN_PAUSE UP    held_ms=",
};
static uint32_t s_down_tick[BTN_COUNT];
static uint8_t  s_glitch_cnt[BTN_COUNT];

static void debug_buttons(uint8_t edges)
{
    uint8_t released, glitch;
    Buttons_DebugInfo(&released, &glitch);
    uint32_t now = (uint32_t)xTaskGetTickCount();

    for (int i = 0; i < BTN_COUNT; i++) {
        uint8_t bit = BTN_BIT(i);
        if ((glitch & bit) && s_glitch_cnt[i] < 255u) s_glitch_cnt[i]++;
        if (edges & bit) {
            s_down_tick[i] = now;
            DBG_LOG1(k_down_msg[i], s_glitch_cnt[i]);
            s_glitch_cnt[i] = 0;
        }
        if (released & bit) {
            DBG_LOG2(k_up_msg[i], (now - s_down_tick[i]) * (uint32_t)portTICK_PERIOD_MS,
                     "  glitch_rejected=", s_glitch_cnt[i]);
            s_glitch_cnt[i] = 0;
        }
    }
}
#endif /* DEBUG_UART */

/* g -> -1..+1 : trừ offset, chia full-scale, bỏ vùng chết */
static float shape(float v)
{
    float t = v / TILT_FULL_SCALE_G;
    if (t > 1.0f) t = 1.0f;
    if (t < -1.0f) t = -1.0f;
    float a = t < 0 ? -t : t;
    if (a < TILT_DEADZONE) return 0.0f;
    a = (a - TILT_DEADZONE) / (1.0f - TILT_DEADZONE);
    return t < 0 ? -a : a;
}

static void calibrate(void)
{
    float sx = 0, sy = 0, gx, gy, gz;
    int n = 0;
    for (int i = 0; i < 40; i++) {                   /* ~0.4 s: giữ board ở tư thế chơi bình thường */
        if (ADXL345_ReadG(&gx, &gy, &gz)) { sx += gx; sy += gy; n++; }
        vTaskDelay(pdMS_TO_TICKS(INPUT_TICK_MS));
    }
    if (n) {
        s_off_x = sx / n; s_off_y = sy / n;
        DBG_LOG2("ADXL345 calib OK, offset_mg x=", (int32_t)(s_off_x * 1000.0f), " y=", (int32_t)(s_off_y * 1000.0f));
    } else {
        DBG_LOG("ADXL345 calib FAIL: khong doc duoc mau nao");
    }
    s_fx = 0; s_fy = 0;
    /* TODO người 1: cho phép hiệu chuẩn lại (vd: giữ nút 3+4 trong 2 giây) */
}

void Input_Task(void *arg)
{
    (void)arg;
    float gx, gy, gz;
    bool adxl_ok = ADXL345_Init();                   /* KHÔNG chặn: không có cảm biến thì nút vẫn chạy */
    int  retry = 0;

    DBG_LOG(adxl_ok ? "ADXL345 init OK"
                    : "ADXL345 init FAIL (thu lai moi ~1 s, nut bam van chay)");
    if (adxl_ok) calibrate();

    TickType_t last = xTaskGetTickCount();
    for (;;) {
        if (adxl_ok) {
            if (ADXL345_ReadG(&gx, &gy, &gz)) {
                s_fx += TILT_FILTER_ALPHA * ((gx - s_off_x) - s_fx);
                s_fy += TILT_FILTER_ALPHA * ((gy - s_off_y) - s_fy);
            }
        } else if (++retry >= 100) {                 /* ~1 s thử lại một lần (cho phép cắm ADXL345 lúc đang chạy) */
            retry = 0;
            adxl_ok = ADXL345_Init();
            if (adxl_ok) {
                DBG_LOG("ADXL345 ket noi lai OK");
                calibrate();
                last = xTaskGetTickCount();
            }
        }

        uint8_t held, edges;
        Buttons_Poll(&held, &edges);
#if DEBUG_UART
        debug_buttons(edges);
#endif

        taskENTER_CRITICAL();
        s_shared.tilt_x  = adxl_ok ? shape(s_fx) * TILT_X_SIGN : 0.0f;
        s_shared.tilt_y  = adxl_ok ? shape(s_fy) * TILT_Y_SIGN : 0.0f;
        s_shared.held    = held;
        s_shared.pressed |= edges;                   /* cộng dồn cho tới khi Game đọc */
        taskEXIT_CRITICAL();

        vTaskDelayUntil(&last, pdMS_TO_TICKS(INPUT_TICK_MS));
    }
}

void Input_Get(InputState_t *out)
{
    taskENTER_CRITICAL();
    *out = s_shared;
    s_shared.pressed = 0;
    taskEXIT_CRITICAL();
}
