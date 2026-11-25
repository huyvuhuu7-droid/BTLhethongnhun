#include "buttons.h"
#include "main.h"
#include "app_config.h"

/* Tên chân đặt trong CubeMX (User Label): BTN_FIRE, BTN_SKILL, BTN_START, BTN_PAUSE
 * -> CubeMX sinh sẵn BTN_FIRE_GPIO_Port / BTN_FIRE_Pin ... Cấu hình GPIO: Input + Pull-up,
 * nút nối chân xuống GND (nhấn = mức thấp). */
typedef struct { GPIO_TypeDef *port; uint16_t pin; } BtnPin_t;

static const BtnPin_t k_pins[BTN_COUNT] = {
    [BTN_FIRE]  = { BTN_FIRE_GPIO_Port,  BTN_FIRE_Pin  },
    [BTN_SKILL] = { BTN_SKILL_GPIO_Port, BTN_SKILL_Pin },
    [BTN_START] = { BTN_START_GPIO_Port, BTN_START_Pin },
    [BTN_PAUSE] = { BTN_PAUSE_GPIO_Port, BTN_PAUSE_Pin },
};

static uint8_t s_cnt[BTN_COUNT];
static uint8_t s_stable;                 /* bit = trạng thái đã ổn định (1 = đang nhấn) */
static uint8_t s_rel;                    /* [debug] bit = nút vừa nhả ở lần Poll gần nhất */
static uint8_t s_glitch;                 /* [debug] bit = có xung nhiễu bị loại ở lần Poll gần nhất */

void Buttons_Poll(uint8_t *held, uint8_t *edges)
{
    uint8_t e = 0;
    s_rel = 0;
    s_glitch = 0;
    for (int i = 0; i < BTN_COUNT; i++) {
        uint8_t bit = (uint8_t)(1u << i);
        uint8_t raw = HAL_GPIO_ReadPin(k_pins[i].port, k_pins[i].pin) == GPIO_PIN_RESET;
        uint8_t cur = (s_stable & bit) ? 1u : 0u;

        if (raw == cur) {
            if (s_cnt[i]) s_glitch |= bit;   /* đang đếm mà mức quay lại như cũ -> xung nhiễu đã bị loại */
            s_cnt[i] = 0;
            continue;
        }
        if (++s_cnt[i] >= BTN_DEBOUNCE_SAMPLES) {
            s_cnt[i] = 0;
            if (raw) { s_stable |= bit; e |= bit; }
            else     { s_stable &= (uint8_t)~bit; s_rel |= bit; }
        }
    }
    *held  = s_stable;
    *edges = e;
}

void Buttons_DebugInfo(uint8_t *released, uint8_t *glitch)
{
    *released = s_rel;
    *glitch   = s_glitch;
}
