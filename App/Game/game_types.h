#ifndef GAME_TYPES_H
#define GAME_TYPES_H
#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"

typedef enum {
    GAME_MENU = 0,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER,
    GAME_WIN
} GameState_t;

/* Vật thể chung: phi thuyền, đạn, địch, thiên thạch, boss. */
typedef struct {
    float   x, y;        /* góc trên-trái (pixel) */
    float   vx, vy;      /* pixel / giây */
    uint8_t w, h;
    int16_t hp;
    int16_t timer_ms;    /* dùng riêng cho từng loại (vd: boss cooldown bắn) */
    bool    active;
} Entity_t;

typedef struct {
    uint16_t enemy_interval_ms;
    uint16_t asteroid_interval_ms;
    float    enemy_speed;
    float    asteroid_speed;
    uint8_t  kills_for_boss;   /* hạ bao nhiêu địch thì boss xuất hiện */
    int16_t  boss_hp;
} LevelCfg_t;

typedef struct {
    GameState_t state;
    uint32_t    frame;              /* đếm tick, dùng làm seed random */

    Entity_t    player;             /* hp nằm trong player.hp */
    uint8_t     mana;
    uint32_t    score;
    uint8_t     level;              /* 0-based */
    uint16_t    kills;              /* số địch đã hạ trong level này */

    /* các bộ đếm lùi (ms) - dùng đếm lùi thay vì timestamp để Pause không bị "nhảy" */
    int16_t     fire_cd_ms;
    int16_t     invincible_ms;
    int16_t     skill_ms;
    int16_t     skill_hit_cd_ms;
    int16_t     enemy_spawn_cd_ms;
    int16_t     asteroid_spawn_cd_ms;

    Entity_t    bullets[MAX_BULLETS];
    Entity_t    enemies[MAX_ENEMIES];
    Entity_t    asteroids[MAX_ASTEROIDS];
    Entity_t    boss_bullets[MAX_BOSS_BULLETS];

    Entity_t    boss;
    bool        boss_active;
    int16_t     boss_hp_max;
} GameData_t;

#endif
