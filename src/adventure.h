#ifndef ADVENTURE_H
#define ADVENTURE_H

#include "player.h"

#define ADV_GOLD_PER_FLOOR       10
#define ADV_STEP_STAMINA         1
#define ADV_FLOOR_STAMINA        5
#define ADV_FLOOR_TIME           1
#define ADV_RELIC_CHANCE         20
#define ADV_RELIC_SELL_PRICE     25
#define ADV_EMERGENCY_GOLD_DIV   2

#define ADV_GREED_GOLD_MULT      2
#define ADV_GREED_STEP_STAMINA   2
#define ADV_SWIFT_FLOOR_STAMINA  1
#define ADV_TROLL_HP_PER_STEP    1

void adventure_run(Player* player, GameState* gs, int is_night);

#endif // ADVENTURE_H