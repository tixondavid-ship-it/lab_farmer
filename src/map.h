#ifndef MAP_H
#define MAP_H

#define MAP_MIN_SIZE        5
#define MAP_MAX_SIZE        30
#define CELL_EMPTY          '.'
#define CELL_TREE           'T'
#define CELL_ROCK           'R'
#define CELL_CAMP           '^'
#define CELL_HERO           '@'
#define CELL_UNKNOWN        '?'
#define OBSTACLE_CHANCE     22
#define MIN_FREE_CELLS      2

#define REVEAL_RADIUS_MIN   5
#define REVEAL_RADIUS_MAX   7
#define REVEAL_STEPS        3

#define MOVE_BLOCKED 0
#define MOVE_OK      1
#define MOVE_EXIT    2

typedef struct {
    char** grid;
    char** revealed;
    int    width;
    int    height;
    int    hero_x, hero_y;
    int    camp_x, camp_y;
    int    floor;
    int    is_night;
    int    reveal_steps_left;
    int    reveal_radius;
} Map;

Map* map_create(int floor, int is_night);
void  map_free(Map** ptr);
void  map_draw(const Map* map);
int   map_move(Map* map, int dx, int dy);
int   map_at_camp(const Map* map);
void  map_shout(Map* map);
void  map_place_hero_at(Map* map, int px, int py);

#endif // MAP_H