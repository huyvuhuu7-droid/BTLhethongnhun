#include "player.h"
#include <string.h>
#include "util.h"

void Player_Reset(GameData_t *g)
{
    Entity_t *p = &g->player;
    memset(p, 0, sizeof(*p));
    p->w = PLAYER_W;  p->h = PLAYER_H;
    p->x = (SCREEN_W - PLAYER_W) / 2.0f;
    p->y = SCREEN_H - PLAYER_H - 4;
    p->hp = PLAYER_MAX_HP;
    p->active = true;
    g->mana = 0;
}

static void fire_bullet(GameData_t *g)
{
    Entity_t *b = Game_AllocEntity(g->bullets, MAX_BULLETS);
    if (!b) return;
    b->w = 1;  b->h = 3;
    b->x = g->player.x + g->player.w / 2.0f - 0.5f;
    b->y = g->player.y - b->h;
    b->vy = -BULLET_SPEED;
    b->hp = 1;
    b->active = true;
}

void Player_Update(GameData_t *g, const InputState_t *in, uint16_t dt_ms)
{
    float dt = dt_ms / 1000.0f;
    Entity_t *p = &g->player;

    /* --- di chuyển có quán tính: nghiêng = gia tốc, ma sát làm chậm dần --- */
    p->vx += in->tilt_x * PLAYER_ACCEL * dt;
    p->vy -= in->tilt_y * PLAYER_ACCEL * dt;        /* y màn hình hướng xuống */
    p->vx -= p->vx * PLAYER_DRAG * dt;
    p->vy -= p->vy * PLAYER_DRAG * dt;
    p->vx = Util_ClampF(p->vx, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
    p->vy = Util_ClampF(p->vy, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
    p->x += p->vx * dt;
    p->y += p->vy * dt;

    if (p->x < 0)                      { p->x = 0;                      p->vx = 0; }
    if (p->x > SCREEN_W - p->w)        { p->x = SCREEN_W - p->w;        p->vx = 0; }
    if (p->y < PLAYER_Y_MIN)           { p->y = PLAYER_Y_MIN;           p->vy = 0; }
    if (p->y > SCREEN_H - p->h)        { p->y = SCREEN_H - p->h;        p->vy = 0; }

    /* --- bộ đếm lùi --- */
    Util_Countdown(&g->fire_cd_ms, dt_ms);
    Util_Countdown(&g->invincible_ms, dt_ms);
    Util_Countdown(&g->skill_ms, dt_ms);
    Util_Countdown(&g->skill_hit_cd_ms, dt_ms);

    /* --- bắn đạn thường (nhấn hoặc giữ nút 1) --- */
    if ((in->held & BTN_BIT(BTN_FIRE)) && g->fire_cd_ms == 0) {
        fire_bullet(g);
        g->fire_cd_ms = FIRE_COOLDOWN_MS;
    }

    /* --- skill: nút 2 khi mana đầy --- */
    if ((in->pressed & BTN_BIT(BTN_SKILL)) && g->mana >= MANA_MAX && g->skill_ms == 0) {
        g->skill_ms = SKILL_DURATION_MS;
        g->skill_hit_cd_ms = 0;
        g->mana = 0;
    }
}
