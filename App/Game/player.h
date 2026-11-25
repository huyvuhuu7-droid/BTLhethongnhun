#ifndef PLAYER_MODULE_H
#define PLAYER_MODULE_H
#include "game.h"
void Player_Reset(GameData_t *g);
void Player_Update(GameData_t *g, const InputState_t *in, uint16_t dt_ms);
#endif
