#ifndef PLAYER_H
#define PLAYER_H

#include "utils.h"

#define INVENTORY_SIZE   10
#define BASE_HP          100
#define BASE_STAMINA     50
#define START_GOLD       50

enum RelicEffect {
    RELIC_NONE = 0,
    RELIC_GREED_RING,
    RELIC_SWIFT_BOOTS,
    RELIC_TROLL_STONE,
    RELIC_COUNT
};

typedef struct {
    enum RelicEffect effect;
    char             name[MAX_NAME_LEN];
} Relic;

typedef struct {
    char   name[MAX_FARMER_NAME_LEN];
    int    hp;
    int    max_hp;
    int    stamina;
    int    max_stamina;
    int    gold;
    int    x, y;
    int    inventory[INVENTORY_SIZE];
    Relic* relic;
} Player;

Player* player_create(const char* name);
void    player_free(Player** ptr);

void    player_print_card(const Player* player, const GameState* gs);
void    player_print_inventory(const Player* player, const GameState* gs);
void    player_put_item(Player* player, const GameState* gs);
void    player_discard_item(Player* player);
void    player_review_resources(const Player* player);
void    player_search_by_name(const Player* player, const GameState* gs);
void    player_rest(Player* player, GameState* gs);
void    player_write_diary(const Player* player, const GameState* gs);

Relic* relic_create(enum RelicEffect effect);
Relic* relic_random(void);
void    relic_free(Relic** ptr);

#endif // PLAYER_H