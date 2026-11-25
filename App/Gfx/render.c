#include "render.h"
#include "gfx.h"
#include "sprites.h"
#include "game.h"
#include "util.h"

/* Không còn phông nền sao. Giữ Render_Init() rỗng để app_rtos.c không phải sửa. */
void Render_Init(void) { }

/* ------------------------------------------------------------ tiện ích */
static int ipos(float v) { return (int)(v < 0 ? v - 0.5f : v + 0.5f); }

static void draw_entity_sprite(const Entity_t *e, const uint8_t *spr, int w, int h)
{
    Gfx_Bitmap(ipos(e->x), ipos(e->y), spr, w, h, 1);
}

static void draw_blink(const char *s, int y, uint32_t now_ms)
{
    if ((now_ms / 400u) % 2u == 0u) Gfx_TextCentered(y, s, 1, 1);
}

/* ------------------------------------------------------------ vẽ thế giới game */
static void draw_world(const GameData_t *g)
{
    /* tia skill: hình chữ nhật đặc, đúng bằng vùng gây sát thương (vẽ trước để vật thể nằm trên) */
    if (g->skill_ms > 0) {
        Entity_t beam;
        Game_SkillBeam(g, &beam);
        Gfx_FillRect(ipos(beam.x), ipos(beam.y), beam.w, beam.h, 1);
    }

    for (int i = 0; i < MAX_ASTEROIDS; i++)
        if (g->asteroids[i].active) draw_entity_sprite(&g->asteroids[i], spr_asteroid, 8, 8);
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (g->enemies[i].active)   draw_entity_sprite(&g->enemies[i], spr_enemy, 8, 8);

    if (g->boss_active) draw_entity_sprite(&g->boss, spr_boss, SPR_BOSS_W, SPR_BOSS_H);

    for (int i = 0; i < MAX_BULLETS; i++) {
        const Entity_t *b = &g->bullets[i];
        if (b->active) Gfx_FillRect(ipos(b->x), ipos(b->y), b->w, b->h, 1);
    }
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
        const Entity_t *b = &g->boss_bullets[i];
        if (b->active) Gfx_FillRect(ipos(b->x), ipos(b->y), b->w, b->h, 1);
    }

    /* phi thuyền nhấp nháy khi đang bất tử */
    if (g->invincible_ms == 0 || (g->invincible_ms / 100) % 2 == 0)
        draw_entity_sprite(&g->player, spr_player, SPR_SHIP_W, SPR_SHIP_H);
}

/* ------------------------------------------------------------ HUD */
static void draw_hud(const GameData_t *g)
{
    char buf[12];
    Gfx_FillRect(0, 0, SCREEN_W, HUD_H, 0);          /* che vật thể đang tràn vào vùng HUD */
    Gfx_HLine(0, HUD_H - 1, SCREEN_W, 1);

    for (int i = 0; i < g->player.hp && i < PLAYER_MAX_HP; i++)   /* máu */
        Gfx_FillRect(i * 5, 1, 4, 4, 1);

    Gfx_DrawRect(30, 1, 40, 5, 1);                                /* mana */
    Gfx_FillRect(31, 2, (38 * g->mana) / MANA_MAX, 3, 1);

    buf[0] = 'L'; Util_U32ToStr((uint32_t)g->level + 1, buf + 1); /* level */
    Gfx_Text(74, 1, buf, 1, 1);

    Util_U32ToStr(g->score, buf);                                 /* điểm (căn phải) */
    Gfx_Text(SCREEN_W - Gfx_TextWidth(buf, 1) - 1, 1, buf, 1, 1);

    if (g->boss_active && g->boss_hp_max > 0) {                   /* thanh máu boss */
        int hp = g->boss.hp > 0 ? g->boss.hp : 0;
        Gfx_DrawRect(34, HUD_H, 60, 3, 1);
        Gfx_FillRect(35, HUD_H + 1, (58 * hp) / g->boss_hp_max, 1, 1);
    }
}

/* ------------------------------------------------------------ các màn hình */
static void draw_menu(uint32_t now_ms)
{
    Gfx_TextCentered(10, "SPACE SHOOTER", 2, 1);
    Gfx_Bitmap((SCREEN_W - 8) / 2, 30, spr_player, 8, 8, 1);
    draw_blink("PRESS START", 48, now_ms);
}

static void draw_end(const GameData_t *g, const char *title, uint32_t now_ms)
{
    char num[12], line[24] = "SCORE ";
    Util_U32ToStr(g->score, num);
    int n = 6; for (int i = 0; num[i]; i++) line[n++] = num[i]; line[n] = '\0';

    Gfx_TextCentered(14, title, 2, 1);
    Gfx_TextCentered(34, line, 1, 1);
    draw_blink("PRESS START", 50, now_ms);
}

void Render_Frame(const GameData_t *g, uint32_t now_ms)
{
    Gfx_Clear();

    switch (g->state) {
    case GAME_MENU:    draw_menu(now_ms); break;
    case GAME_PLAYING: draw_world(g); draw_hud(g); break;
    case GAME_PAUSED:
        draw_world(g); draw_hud(g);
        Gfx_FillRect(36, 24, 56, 16, 0);
        Gfx_DrawRect(36, 24, 56, 16, 1);
        Gfx_TextCentered(29, "PAUSED", 2, 1);
        break;
    case GAME_OVER:    draw_end(g, "GAME OVER", now_ms); break;
    case GAME_WIN:     draw_end(g, "YOU WIN", now_ms);   break;
    }
}
