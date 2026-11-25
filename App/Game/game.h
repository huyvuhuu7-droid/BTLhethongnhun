/* game.h - API công khai của logic game. KHÔNG include HAL/FreeRTOS ở thư mục Game/
 * để có thể biên dịch và chạy thử trên PC (xem tests/). */
#ifndef GAME_H
#define GAME_H
#include "game_types.h"
#include "input_types.h"

void Game_Init(GameData_t *g);                        /* về màn hình MENU */
void Game_Reset(GameData_t *g);                       /* bắt đầu ván mới -> PLAYING */
void Game_Update(GameData_t *g, const InputState_t *in, uint16_t dt_ms);

/* ---- helper dùng chung cho các module trong Game/ và Gfx/ ---- */
bool       Game_Overlap(const Entity_t *a, const Entity_t *b);
Entity_t  *Game_AllocEntity(Entity_t *pool, uint8_t n); /* NULL nếu pool đầy; caller tự set active */
void       Game_AddScore(GameData_t *g, uint32_t pts);
void       Game_AddMana(GameData_t *g, uint8_t amount);
void       Game_HurtPlayer(GameData_t *g, int dmg);
void       Game_SkillBeam(const GameData_t *g, Entity_t *out); /* hình chữ nhật của tia skill */

#endif
