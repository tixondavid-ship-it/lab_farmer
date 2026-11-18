#include "map.h"
#include "utils.h"

static void free_grid(char** grid, int rows) {
    int i;
    if (!grid) return;
    for (i = 0; i < rows; i++) free(grid[i]);
    free(grid);
}

Map* map_create(int floor, int is_night) {
    int i, j, empty_count;
    Map* m = (Map*)malloc(sizeof(Map));
    if (!m) return NULL;
    memset(m, 0, sizeof(*m));

    m->width = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    m->height = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    m->floor = floor;
    m->is_night = is_night;
    m->reveal_steps_left = 0;
    m->reveal_radius = REVEAL_RADIUS_MIN +
        rand() % (REVEAL_RADIUS_MAX - REVEAL_RADIUS_MIN + 1);

    m->grid = (char**)calloc((size_t)m->height, sizeof(char*));
    m->revealed = (char**)calloc((size_t)m->height, sizeof(char*));
    if (!m->grid || !m->revealed) {
        free_grid(m->grid, m->height);
        free_grid(m->revealed, m->height);
        free(m);
        return NULL;
    }
    for (i = 0; i < m->height; i++) {
        m->grid[i] = (char*)malloc((size_t)m->width * sizeof(char));
        m->revealed[i] = (char*)calloc((size_t)m->width, sizeof(char));
        if (!m->grid[i] || !m->revealed[i]) {
            free_grid(m->grid, m->height);
            free_grid(m->revealed, m->height);
            free(m);
            return NULL;
        }
    }

    for (i = 0; i < m->height; i++) {
        for (j = 0; j < m->width; j++) {
            if (rand() % 100 < OBSTACLE_CHANCE)
                m->grid[i][j] = (rand() % 2 == 0) ? CELL_TREE : CELL_ROCK;
            else
                m->grid[i][j] = CELL_EMPTY;
        }
    }

    empty_count = 0;
    for (i = 0; i < m->height; i++)
        for (j = 0; j < m->width; j++)
            if (m->grid[i][j] == CELL_EMPTY) empty_count++;

    while (empty_count < MIN_FREE_CELLS) {
        i = rand() % m->height;
        j = rand() % m->width;
        if (m->grid[i][j] != CELL_EMPTY) {
            m->grid[i][j] = CELL_EMPTY;
            empty_count++;
        }
    }

    do {
        m->camp_x = rand() % m->width;
        m->camp_y = rand() % m->height;
    } while (m->grid[m->camp_y][m->camp_x] != CELL_EMPTY);
    m->grid[m->camp_y][m->camp_x] = CELL_CAMP;

    do {
        m->hero_x = rand() % m->width;
        m->hero_y = rand() % m->height;
    } while (m->grid[m->hero_y][m->hero_x] != CELL_EMPTY);

    return m;
}

void map_free(Map** ptr) {
    Map* m;
    if (!ptr || !*ptr) return;
    m = *ptr;
    free_grid(m->grid, m->height);
    free_grid(m->revealed, m->height);
    debug_free_message(m);
    free(m);
    *ptr = NULL;
}

void map_draw(const Map* map) {
    int i, j;
    printf("\n+");
    for (j = 0; j < map->width; j++) printf("-");
    printf("+\n");
    for (i = 0; i < map->height; i++) {
        printf("|");
        for (j = 0; j < map->width; j++) {
            char c;
            if (i == map->hero_y && j == map->hero_x)
                c = CELL_HERO;
            else if (map->is_night && !map->revealed[i][j]
                && map->grid[i][j] != CELL_CAMP)
                c = CELL_UNKNOWN;
            else
                c = map->grid[i][j];
            putchar(c);
        }
        printf("|\n");
    }
    printf("+");
    for (j = 0; j < map->width; j++) printf("-");
    printf("+\n");
}

int map_move(Map* map, int dx, int dy) {
    int nx = map->hero_x + dx;
    int ny = map->hero_y + dy;
    char c;
    if (nx < 0 || nx >= map->width || ny < 0 || ny >= map->height)
        return MOVE_EXIT;
    c = map->grid[ny][nx];
    if (c == CELL_TREE || c == CELL_ROCK)
        return MOVE_BLOCKED;
    map->hero_x = nx;
    map->hero_y = ny;
    return MOVE_OK;
}

int map_at_camp(const Map* map) {
    return map->hero_x == map->camp_x && map->hero_y == map->camp_y;
}

void map_shout(Map* map) {
    int i, j, r;
    if (!map->is_night) {
        printf("Днём кричать бессмысленно.\n");
        return;
    }
    r = map->reveal_radius;
    for (i = -r; i <= r; i++)
        for (j = -r; j <= r; j++) {
            int y, x;
            if (i * i + j * j > r * r) continue;
            y = map->hero_y + i;
            x = map->hero_x + j;
            if (y >= 0 && y < map->height && x >= 0 && x < map->width)
                map->revealed[y][x] = 1;
        }
    map->reveal_steps_left = REVEAL_STEPS;
    printf("Вы кричите! Окрестности озаряются на %d шагов.\n", REVEAL_STEPS);
}

void map_place_hero_at(Map* map, int px, int py) {
    int best_d = INT_MAX, bx = -1, by = -1, x, y;
    for (y = 0; y < map->height; y++)
        for (x = 0; x < map->width; x++) {
            if (map->grid[y][x] == CELL_EMPTY || map->grid[y][x] == CELL_CAMP) {
                int d = abs(x - px) + abs(y - py);
                if (d < best_d) { best_d = d; bx = x; by = y; }
            }
        }
    if (bx >= 0) { map->hero_x = bx; map->hero_y = by; }
}