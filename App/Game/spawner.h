#ifndef SPAWNER_H
#define SPAWNER_H
#include "game.h"
void              Spawner_Reset(GameData_t *g);
void              Spawner_Update(GameData_t *g, uint16_t dt_ms);   /* sinh + di chuyển địch/thiên thạch */
const LevelCfg_t *Spawner_Cfg(uint8_t level);
#endif
