/* input_types.h - "hợp đồng" giữa Input (người 1) và Game (người 2). */
#ifndef INPUT_TYPES_H
#define INPUT_TYPES_H
#include <stdint.h>

typedef enum {
    BTN_FIRE = 0,   /* Nút 1: bắn đạn (nhấn hoặc giữ) */
    BTN_SKILL,      /* Nút 2: skill khi đầy mana */
    BTN_START,      /* Nút 3: start / restart */
    BTN_PAUSE,      /* Nút 4: pause / resume */
    BTN_COUNT
} Button_t;

#define BTN_BIT(b)  ((uint8_t)(1u << (b)))

typedef struct {
    float   tilt_x;    /* -1..+1 ; +1 = nghiêng sang phải */
    float   tilt_y;    /* -1..+1 ; +1 = nghiêng về trước (phi thuyền đi lên) */
    uint8_t held;      /* bit = nút đang giữ */
    uint8_t pressed;   /* bit = nút vừa được nhấn kể từ lần đọc trước (cạnh xuống) */
} InputState_t;

#endif
