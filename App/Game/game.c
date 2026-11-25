#include "game.h"
#include <string.h>
#include "player.h"
#include "spawner.h"
#include "boss.h"
#include "util.h"

/* ------------------------------------------------------------------ helpers */
bool Game_Overlap(const Entity_t *a, const Entity_t *b)
{
    return a->x < b->x + b->w && a->x + a->w > b->x &&
           a->y < b->y + b->h && a->y + a->h > b->y;
}

Entity_t *Game_AllocEntity(Entity_t *pool, uint8_t n)
{
    for (uint8_t i = 0; i < n; i++) {
        if (!pool[i].active) {
            memset(&pool[i], 0, sizeof(Entity_t));
            return &pool[i];
        }
    }
    return NULL;
}

void Game_AddScore(GameData_t *g, uint32_t pts) { g->score += pts; }

void Game_AddMana(GameData_t *g, uint8_t amount)
{
    uint16_t m = (uint16_t)g->mana + amount;
    g->mana = (uint8_t)(m > MANA_MAX ? MANA_MAX : m);
}

void Game_HurtPlayer(GameData_t *g, int dmg)
{
    if (g->invincible_ms > 0) return;
    g->player.hp = (int16_t)(g->player.hp - dmg);
    g->invincible_ms = INVINCIBLE_MS;
}

void Game_SkillBeam(const GameData_t *g, Entity_t *out)
{
    memset(out, 0, sizeof(*out));
    out->w = SKILL_WIDTH;
    out->x = g->player.x + g->player.w / 2.0f - SKILL_WIDTH / 2.0f;
    out->y = PLAYFIELD_TOP;
    out->h = (uint8_t)(g->player.y > PLAYFIELD_TOP ? g->player.y - PLAYFIELD_TOP : 0);
    out->active = true;
}

/* ------------------------------------------------------------------ lifecycle */
void Game_Init(GameData_t *g)
{
    memset(g, 0, sizeof(*g));
    g->state = GAME_MENU;
}

void Game_Reset(GameData_t *g)
{
    uint32_t f = g->frame;                 /* giữ frame để làm seed ngẫu nhiên */
    memset(g, 0, sizeof(*g));
    g->frame = f;
    Rand_Seed(f * 2654435761u + 1u);
    g->state = GAME_PLAYING;
    Player_Reset(g);
    Spawner_Reset(g);
}

/* ------------------------------------------------------------------ chuyển động đạn */
static void move_projectiles(Entity_t *pool, uint8_t n, float dt)
{
    for (uint8_t i = 0; i < n; i++) {
        Entity_t *e = &pool[i];
        if (!e->active) continue;
        e->x += e->vx * dt;
        e->y += e->vy * dt;
        if (e->y + e->h < 0 || e->y > SCREEN_H || e->x + e->w < 0 || e->x > SCREEN_W)
            e->active = false;
    }
}

/* ------------------------------------------------------------------ sát thương */
static void damage_enemy(GameData_t *g, Entity_t *e, int dmg)
{
    e->hp = (int16_t)(e->hp - dmg);
    if (e->hp <= 0) {
        e->active = false;
        Game_AddScore(g, SCORE_ENEMY);
        Game_AddMana(g, MANA_PER_KILL);
        g->kills++;
    }
}

static void damage_asteroid(GameData_t *g, Entity_t *e, int dmg)
{
    e->hp = (int16_t)(e->hp - dmg);
    if (e->hp <= 0) {                       /* thiên thạch: có điểm, không có mana */
        e->active = false;
        Game_AddScore(g, SCORE_ASTEROID);
    }
}

/* Trả về true nếu đạn đã trúng thứ gì đó */
static bool bullet_hit(GameData_t *g, Entity_t *b)
{
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        Entity_t *e = &g->enemies[i];
        if (e->active && Game_Overlap(b, e)) { damage_enemy(g, e, 1); return true; }
    }
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        Entity_t *a = &g->asteroids[i];
        if (a->active && Game_Overlap(b, a)) { damage_asteroid(g, a, 1); return true; }
    }
    if (g->boss_active && Game_Overlap(b, &g->boss)) { g->boss.hp--; return true; }
    return false;
}

static void collisions(GameData_t *g)
{
    /* 1. đạn của người chơi */
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        Entity_t *b = &g->bullets[i];
        if (b->active && bullet_hit(g, b)) b->active = false;
    }

    /* 2. tia skill (gây sát thương theo nhịp) */
    if (g->skill_ms > 0 && g->skill_hit_cd_ms == 0) {
        Entity_t beam;
        Game_SkillBeam(g, &beam);
        g->skill_hit_cd_ms = SKILL_HIT_INTERVAL_MS;
        for (uint8_t i = 0; i < MAX_ENEMIES; i++)
            if (g->enemies[i].active && Game_Overlap(&beam, &g->enemies[i]))
                damage_enemy(g, &g->enemies[i], SKILL_DAMAGE);
        for (uint8_t i = 0; i < MAX_ASTEROIDS; i++)
            if (g->asteroids[i].active && Game_Overlap(&beam, &g->asteroids[i]))
                damage_asteroid(g, &g->asteroids[i], SKILL_DAMAGE);
        for (uint8_t i = 0; i < MAX_BOSS_BULLETS; i++)
            if (g->boss_bullets[i].active && Game_Overlap(&beam, &g->boss_bullets[i]))
                g->boss_bullets[i].active = false;
        if (g->boss_active && Game_Overlap(&beam, &g->boss))
            g->boss.hp = (int16_t)(g->boss.hp - SKILL_DAMAGE);
    }

    /* 3. người chơi bị trúng */
    for (uint8_t i = 0; i < MAX_ENEMIES; i++) {
        Entity_t *e = &g->enemies[i];
        if (e->active && Game_Overlap(&g->player, e)) { e->active = false; Game_HurtPlayer(g, 1); }
    }
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        Entity_t *a = &g->asteroids[i];
        if (a->active && Game_Overlap(&g->player, a)) { a->active = false; Game_HurtPlayer(g, 1); }
    }
    for (uint8_t i = 0; i < MAX_BOSS_BULLETS; i++) {
        Entity_t *b = &g->boss_bullets[i];
        if (b->active && Game_Overlap(&g->player, b)) { b->active = false; Game_HurtPlayer(g, 1); }
    }
    if (g->boss_active && Game_Overlap(&g->player, &g->boss)) Game_HurtPlayer(g, 1);
}

/* ------------------------------------------------------------------ vòng lặp chính */
static void step_playing(GameData_t *g, const InputState_t *in, uint16_t dt_ms)
{
    float dt = dt_ms / 1000.0f;

    Player_Update(g, in, dt_ms);
    Spawner_Update(g, dt_ms);
    Boss_Update(g, dt_ms);
    if (g->state != GAME_PLAYING) return;            /* boss chết ở màn cuối -> WIN */

    move_projectiles(g->bullets, MAX_BULLETS, dt);
    move_projectiles(g->boss_bullets, MAX_BOSS_BULLETS, dt);
    collisions(g);

    if (g->player.hp <= 0) g->state = GAME_OVER;
}

void Game_Update(GameData_t *g, const InputState_t *in, uint16_t dt_ms)
{
    g->frame++;
    switch (g->state) {
    case GAME_MENU:
        if (in->pressed & BTN_BIT(BTN_START)) Game_Reset(g);
        break;
    case GAME_PLAYING:
        if (in->pressed & BTN_BIT(BTN_PAUSE)) { g->state = GAME_PAUSED; break; }
        step_playing(g, in, dt_ms);
        break;
    case GAME_PAUSED:
        if (in->pressed & BTN_BIT(BTN_PAUSE)) g->state = GAME_PLAYING;
        break;
    case GAME_OVER:
    case GAME_WIN:
        if (in->pressed & BTN_BIT(BTN_START)) Game_Reset(g);
        break;
    }
}
