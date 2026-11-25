#include "spawner.h"
#include <string.h>
#include "boss.h"
#include "util.h"

/* Bảng độ khó từng màn - sửa ở đây để cân bằng game. */
static const LevelCfg_t k_levels[LEVELS_TOTAL] = {
    /* enemy_ms, asteroid_ms, e_speed, a_speed, kills->boss, boss_hp */
    { 1400, 2500, 18.0f, 14.0f,  8,  30 },
    { 1100, 2000, 24.0f, 18.0f, 12,  50 },
    {  800, 1600, 30.0f, 22.0f, 16,  80 },
};

const LevelCfg_t *Spawner_Cfg(uint8_t level)
{
    if (level >= LEVELS_TOTAL) level = LEVELS_TOTAL - 1;
    return &k_levels[level];
}

void Spawner_Reset(GameData_t *g)
{
    g->enemy_spawn_cd_ms    = 1000;
    g->asteroid_spawn_cd_ms = 2000;
    g->kills = 0;
}

static void spawn_enemy(GameData_t *g, const LevelCfg_t *c)
{
    Entity_t *e = Game_AllocEntity(g->enemies, MAX_ENEMIES);
    if (!e) return;
    e->w = 8;  e->h = 8;
    e->x = (float)Rand_Range(0, SCREEN_W - e->w);
    e->y = -(float)e->h + PLAYFIELD_TOP;
    e->vx = (float)Rand_Range(-10, 10);
    e->vy = c->enemy_speed;
    e->hp = 1;
    e->active = true;
}

static void spawn_asteroid(GameData_t *g, const LevelCfg_t *c)
{
    Entity_t *a = Game_AllocEntity(g->asteroids, MAX_ASTEROIDS);
    if (!a) return;
    a->w = 8;  a->h = 8;
    a->x = (float)Rand_Range(0, SCREEN_W - a->w);
    a->y = -(float)a->h + PLAYFIELD_TOP;
    a->vx = (float)Rand_Range(-4, 4);
    a->vy = c->asteroid_speed;
    a->hp = 3;                     /* TODO nhóm: thiên thạch có bắn vỡ được không? */
    a->active = true;
}

static void move_pool(Entity_t *pool, uint8_t n, float dt, bool bounce)
{
    for (uint8_t i = 0; i < n; i++) {
        Entity_t *e = &pool[i];
        if (!e->active) continue;
        e->x += e->vx * dt;
        e->y += e->vy * dt;
        if (bounce) {
            if (e->x < 0)                 { e->x = 0;                 e->vx = -e->vx; }
            if (e->x > SCREEN_W - e->w)   { e->x = SCREEN_W - e->w;   e->vx = -e->vx; }
        }
        if (e->y > SCREEN_H) e->active = false;
    }
}

void Spawner_Update(GameData_t *g, uint16_t dt_ms)
{
    const LevelCfg_t *c = Spawner_Cfg(g->level);
    float dt = dt_ms / 1000.0f;

    if (!g->boss_active) {
        if (g->kills >= c->kills_for_boss) {
            Boss_Spawn(g);                               /* đủ điểm -> boss cuối màn */
        } else {
            Util_Countdown(&g->enemy_spawn_cd_ms, dt_ms);
            if (g->enemy_spawn_cd_ms == 0) {
                spawn_enemy(g, c);
                g->enemy_spawn_cd_ms = (int16_t)c->enemy_interval_ms;
            }
            Util_Countdown(&g->asteroid_spawn_cd_ms, dt_ms);
            if (g->asteroid_spawn_cd_ms == 0) {
                spawn_asteroid(g, c);
                g->asteroid_spawn_cd_ms = (int16_t)c->asteroid_interval_ms;
            }
        }
    }

    move_pool(g->enemies,   MAX_ENEMIES,   dt, true);
    move_pool(g->asteroids, MAX_ASTEROIDS, dt, false);
}
