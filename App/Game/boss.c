#include "boss.h"
#include <string.h>
#include "spawner.h"
#include "util.h"

void Boss_Spawn(GameData_t *g)
{
    const LevelCfg_t *c = Spawner_Cfg(g->level);
    Entity_t *b = &g->boss;
    memset(b, 0, sizeof(*b));
    b->w = BOSS_W;  b->h = BOSS_H;
    b->x = (SCREEN_W - BOSS_W) / 2.0f;
    b->y = (float)PLAYFIELD_TOP - BOSS_H;               /* bay vào từ trên */
    b->vx = BOSS_SPEED;
    b->hp = c->boss_hp;
    b->timer_ms = BOSS_FIRST_SHOT_MS;
    b->active = true;
    g->boss_hp_max = c->boss_hp;
    g->boss_active = true;
}

/* Số viên mỗi loạt tăng theo level: 1 -> 3 -> 5 (đạn tỏa quạt).
 * TODO nhóm: thêm pattern khác (đạn nhắm vào người chơi, xoáy...). */
static void boss_fire(GameData_t *g)
{
    Entity_t *b = &g->boss;
    int n = g->level;
    float cx = b->x + b->w / 2.0f - 1.0f;
    for (int i = -n; i <= n; i++) {
        Entity_t *s = Game_AllocEntity(g->boss_bullets, MAX_BOSS_BULLETS);
        if (!s) break;
        s->w = 2;  s->h = 2;
        s->x = cx;
        s->y = b->y + b->h;
        s->vx = i * 14.0f;
        s->vy = BOSS_BULLET_SPEED;
        s->hp = 1;
        s->active = true;
    }
}

static void boss_defeated(GameData_t *g)
{
    Game_AddScore(g, SCORE_BOSS * (uint32_t)(g->level + 1));
    memset(g->boss_bullets, 0, sizeof(g->boss_bullets));
    memset(g->enemies,      0, sizeof(g->enemies));
    memset(g->asteroids,    0, sizeof(g->asteroids));
    g->boss.active = false;
    g->boss_active = false;

    if (g->level + 1 >= LEVELS_TOTAL) {                 /* hạ boss cuối -> thắng */
        g->state = GAME_WIN;
        return;
    }
    g->level++;
    g->kills = 0;
    g->enemy_spawn_cd_ms = 1500;
    g->asteroid_spawn_cd_ms = 2500;
    if (g->player.hp < PLAYER_MAX_HP) g->player.hp++;   /* thưởng 1 máu qua màn */
}

void Boss_Update(GameData_t *g, uint16_t dt_ms)
{
    if (!g->boss_active) return;
    Entity_t *b = &g->boss;
    float dt = dt_ms / 1000.0f;

    if (b->hp <= 0) { boss_defeated(g); return; }

    if (b->y < BOSS_Y) {                                /* đang bay vào, chưa bắn */
        b->y += BOSS_ENTER_SPEED * dt;
        if (b->y > BOSS_Y) b->y = BOSS_Y;
        return;
    }

    b->x += b->vx * dt;
    if (b->x < 0)               { b->x = 0;               b->vx = -b->vx; }
    if (b->x > SCREEN_W - b->w) { b->x = SCREEN_W - b->w; b->vx = -b->vx; }

    Util_Countdown(&b->timer_ms, dt_ms);
    if (b->timer_ms == 0) {
        boss_fire(g);
        int16_t interval = (int16_t)(BOSS_FIRE_MS_BASE - g->level * BOSS_FIRE_MS_STEP);
        b->timer_ms = interval;
    }
}
