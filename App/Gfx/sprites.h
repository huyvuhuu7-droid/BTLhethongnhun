/* sprites.h - bitmap 1-bit. TODO người 3: vẽ lại đẹp hơn (dùng image2cpp, chế độ horizontal). */
#ifndef SPRITES_H
#define SPRITES_H
#include <stdint.h>

#define SPR_SHIP_W 8
#define SPR_SHIP_H 8
extern const uint8_t spr_player[8];
extern const uint8_t spr_enemy[8];
extern const uint8_t spr_asteroid[8];

#define SPR_BOSS_W 24
#define SPR_BOSS_H 12
extern const uint8_t spr_boss[36];

#endif
