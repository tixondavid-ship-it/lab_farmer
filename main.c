#define _CRT_SECURE_NO_WARNINGS
#pragma execution_character_set("utf-8")
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define INVENTORY_SIZE 10
#define MAX_ITEMS 10
#define MAX_NAME_LEN 64
#define MAX_FARMER_NAME_LEN 33
#define MAX_LINE_LEN 256

#define MIN_ITEM_ID 0
#define MAX_ITEM_ID (MAX_ITEMS - 1)

#define MAP_MIN_SIZE        5
#define MAP_MAX_SIZE        30
#define CELL_EMPTY          '.'
#define CELL_TREE           'T'
#define CELL_ROCK           'R'
#define CELL_CAMP           '^'
#define CELL_HERO           '@'
#define OBSTACLE_CHANCE     22          
#define MIN_FREE_CELLS      2

#define MOVE_BLOCKED 0
#define MOVE_OK      1
#define MOVE_EXIT    2

static const char* DEFAULT_ITEM_NAMES[MAX_ITEMS] = {
    "Пусто", "Дерево", "Камень", "Семена", "Хлеб",
    "Уголь", "Саженец", "Удобрение", "Ткань", "Факел"
};

typedef struct {
    char** grid;
    int    width;
    int    height;
    int    hero_x, hero_y;
    int    camp_x, camp_y;
    int    distance;
} ForestMap;

ForestMap* forest_create(int distance);
void       forest_free(ForestMap* map);
void       forest_draw(const ForestMap* map);
void       forest_place_hero_at(ForestMap* map, int px, int py);
int        forest_move(ForestMap* map, int dx, int dy); 
int        forest_at_camp(const ForestMap* map);        
void       adventure_run(const char* farmer_name);

int  read_int(const char* prompt, int* value);
int  read_line(const char* prompt, char* buffer, size_t buffer_size);
void trim_newline(char* text);
void load_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void copy_default_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void print_menu(const char* farmer_name);
void print_inventory(const int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void show_time(int current_day, int current_hour);
void work(int* current_day, int* current_hour);
void put_item(int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void discard_item(int inventory[INVENTORY_SIZE]);
void review_resources(const int inventory[INVENTORY_SIZE]);
void search_item_by_name(const int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void write_diary(const char* farmer_name, int current_day, int current_hour, const int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void summarize_deliveries(void);

int main(void) {
    SetConsoleOutputCP(CP_UTF8);
    srand((unsigned)time(NULL));

    char farmer_name[MAX_FARMER_NAME_LEN];
    char item_names[MAX_ITEMS][MAX_NAME_LEN];

    int inventory[INVENTORY_SIZE] = { 3, 1, 2, 4, 3, 5, 1, 6, 0, 1 };
    int current_day = 1;
    int current_hour = 8;
    int action;

    printf("Введите имя фермера: ");
    if (!read_line("", farmer_name, sizeof(farmer_name)) || farmer_name[0] == '\0') {
        strcpy_s(farmer_name, sizeof(farmer_name), "Фермер");
    }

    load_item_names(item_names);

    do {
        print_menu(farmer_name);
        if (!read_int("Выберите пункт: ", &action)) {
            printf("Введите целое число от 0 до 10.\n");
            continue;
        }

        switch (action) {
        case 0:
            printf("Пока-пока, %s!\n", farmer_name);
            break;
        case 1: show_time(current_day, current_hour); break;
        case 2: work(&current_day, &current_hour);    break;
        case 3: print_inventory(inventory, item_names); break;
        case 4: put_item(inventory, item_names);      break;
        case 5: discard_item(inventory);              break;
        case 6: review_resources(inventory);          break;
        case 7: search_item_by_name(inventory, item_names); break;
        case 8:
            write_diary(farmer_name, current_day, current_hour, inventory, item_names);
            break;
        case 9: summarize_deliveries(); break;
        case 10: adventure_run(farmer_name); break;
        default:
            printf("Выберите пункт от 0 до 10.\n");
            break;
        }
    } while (action != 0);

    return 0;
}

int read_int(const char* prompt, int* value) {
    char line[MAX_LINE_LEN];
    char extra;
    int  result;

    printf("%s", prompt);
    if (!fgets(line, sizeof(line), stdin)) return 0;

    result = sscanf_s(line, " %d %c", value, &extra, 1);
    return result == 1;
}

int read_line(const char* prompt, char* buffer, size_t buffer_size) {
    char    raw[MAX_LINE_LEN];
    wchar_t wbuf[MAX_LINE_LEN];
    UINT    cp;
    size_t  n;
    int     wlen, ulen;

    printf("%s", prompt);
    fflush(stdout);
    buffer[0] = '\0';

    if (!fgets(raw, sizeof(raw), stdin)) return 0;

    n = strlen(raw);
    while (n > 0 && (raw[n - 1] == '\n' || raw[n - 1] == '\r'))
        raw[--n] = '\0';
    if (n == 0) return 1;

    cp = GetConsoleCP();
    if (cp == CP_UTF8) {
        strncpy_s(buffer, buffer_size, raw, _TRUNCATE);
        return 1;
    }

    wlen = MultiByteToWideChar(cp, 0, raw, -1, wbuf, (int)(sizeof(wbuf) / sizeof(wbuf[0])));
    if (wlen <= 0) return 0;

    ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buffer, (int)buffer_size, NULL, NULL);
    return ulen > 0;
}

void trim_newline(char* text) {
    size_t length = strlen(text);
    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[--length] = '\0';
    }
}

void copy_default_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int i;
    for (i = 0; i < MAX_ITEMS; i++)
        snprintf(item_names[i], MAX_NAME_LEN, "%s", DEFAULT_ITEM_NAMES[i]);
}

void load_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* file = NULL;
    int id;
    char name[MAX_NAME_LEN];
    int loaded_count = 0;

    copy_default_item_names(item_names);

    if (fopen_s(&file, "items.txt", "r") != 0) file = NULL;
    if (file == NULL) {
        printf("Предупреждение: items.txt не найден. Используются стандартные названия.\n");
        return;
    }

    while (fscanf_s(file, "%d %63s", &id, name, (unsigned)sizeof(name)) == 2) {
        if (id >= MIN_ITEM_ID && id <= MAX_ITEM_ID) {
            snprintf(item_names[id], MAX_NAME_LEN, "%s", name);
            loaded_count++;
        }
    }
    fclose(file);

    if (loaded_count == 0) {
        printf("Предупреждение: в items.txt не найдено корректных записей.\n");
    }
}

void print_menu(const char* farmer_name) {
    printf("\n===== ФЕРМА: %s =====\n", farmer_name);
    printf("[0] Выход\n");
    printf("[1] Посмотреть на часы\n");
    printf("[2] Промотать время (поработать)\n");
    printf("[3] Посмотреть инвентарь\n");
    printf("[4] Положить предмет в слот\n");
    printf("[5] Выбросить предмет\n");
    printf("[6] Ревизия ресурсов по ID\n");
    printf("[7] Поиск предмета в рюкзаке по названию\n");
    printf("[8] Записать состояние в дневник\n");
    printf("[9] Подсчитать ресурсы из input.txt\n");
    printf("[10] Отправиться в приключение\n");
}

void show_time(int current_day, int current_hour) {
    printf("Текущее время: %d день, %02d:00\n", current_day, current_hour);
}

void work(int* current_day, int* current_hour) {
    int hours;
    if (!read_int("Сколько часов работать? ", &hours)) {
        printf("Введите целое число.\n"); return;
    }
    if (hours < 0) {
        printf("Количество часов не может быть отрицательным.\n"); return;
    }
    if (hours > INT_MAX - *current_hour) {
        printf("Слишком большое количество часов.\n"); return;
    }
    *current_hour += hours;
    *current_day += *current_hour / 24;
    *current_hour %= 24;
    show_time(*current_day, *current_hour);
}

void print_inventory(const int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int i;
    printf("\n===== ИНВЕНТАРЬ =====\n");
    for (i = 0; i < INVENTORY_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n", i, inventory[i], item_names[inventory[i]]);
    }
}

void put_item(int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int slot, item_id;
    if (!read_int("Введите индекс слота от 0 до 9: ", &slot)) {
        printf("Некорректный номер слота.\n"); return;
    }
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Такого слота не существует.\n"); return;
    }
    if (!read_int("Введите ID предмета от 0 до 9: ", &item_id)) {
        printf("Некорректный ID.\n"); return;
    }
    if (item_id < MIN_ITEM_ID || item_id > MAX_ITEM_ID) {
        printf("ID должен быть от 0 до 9.\n"); return;
    }
    inventory[slot] = item_id;
    printf("В слот %d помещён предмет: %s.\n", slot, item_names[item_id]);
}

void discard_item(int inventory[INVENTORY_SIZE]) {
    int slot;
    if (!read_int("Введите номер слота от 0 до 9: ", &slot)) {
        printf("Некорректный номер слота.\n"); return;
    }
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Такого слота не существует.\n"); return;
    }
    inventory[slot] = 0;
    printf("Слот %d очищен.\n", slot);
}

void review_resources(const int inventory[INVENTORY_SIZE]) {
    int searched_id, count = 0, i;
    if (!read_int("Введите ID предмета для поиска: ", &searched_id)) {
        printf("Некорректный ID.\n"); return;
    }
    if (searched_id < MIN_ITEM_ID || searched_id > MAX_ITEM_ID) {
        printf("ID должен быть от 0 до 9.\n"); return;
    }
    printf("Позиции предмета: ");
    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == searched_id) { printf("%d ", i); count++; }
    }
    printf("\nКоличество предметов с ID %d: %d\n", searched_id, count);
    if (count == 0) printf("Такой предмет не найден.\n");
}

void search_item_by_name(const int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    char searched_name[MAX_NAME_LEN];
    int  item_id = -1, count = 0, i;

    if (!read_line("Введите название предмета: ", searched_name, sizeof(searched_name))) {
        printf("Не удалось прочитать название.\n"); return;
    }

    for (i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(searched_name, item_names[i]) == 0) {
            item_id = i; break;
        }
    }
    if (item_id == -1) {
        printf("Такого названия нет в каталоге.\n"); return;
    }

    printf("Предмет найден в слотах: ");
    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == item_id) { printf("%d ", i); count++; }
    }
    if (count == 0) printf("не найден");
    printf("\n");
}

void write_diary(const char* farmer_name, int current_day, int current_hour, const int inventory[INVENTORY_SIZE], const char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* file = NULL;
    int i;

    if (fopen_s(&file, "diary.txt", "a") != 0) file = NULL;
    if (file == NULL) {
        printf("Не удалось открыть diary.txt для записи.\n"); return;
    }
    fprintf(file, "===== Дневник фермера =====\n");
    fprintf(file, "Фермер: %s\n", farmer_name);
    fprintf(file, "День: %d, время: %02d:00\n", current_day, current_hour);
    fprintf(file, "Инвентарь:\n");
    for (i = 0; i < INVENTORY_SIZE; i++)
        fprintf(file, "Слот %d: %s\n", i, item_names[inventory[i]]);
    fprintf(file, "\n");
    fclose(file);
    printf("Запись добавлена в diary.txt.\n");
}

void summarize_deliveries(void) {
    FILE* input = NULL;
    FILE* output = NULL;
    char names[MAX_ITEMS][MAX_NAME_LEN];
    int  totals[MAX_ITEMS] = { 0 };
    int  unique_count = 0;
    char name[MAX_NAME_LEN];
    int  quantity, i, valid_records = 0;

    if (fopen_s(&input, "input.txt", "r") != 0) input = NULL;
    if (input == NULL) {
        printf("Не удалось открыть input.txt.\n"); return;
    }
    while (fscanf_s(input, "%63s %d", name, (unsigned)sizeof(name), &quantity) == 2) {
        if (quantity < 0) {
            printf("Пропущена запись с отрицательным количеством: %s %d\n", name, quantity);
            continue;
        }
        for (i = 0; i < unique_count; i++)
            if (strcmp(names[i], name) == 0) break;
        if (i == unique_count) {
            if (unique_count >= MAX_ITEMS) {
                printf("Слишком много разных ресурсов.\n");
                fclose(input); return;
            }
            snprintf(names[unique_count], MAX_NAME_LEN, "%s", name);
            totals[unique_count] = 0;
            unique_count++;
        }
        if (quantity > INT_MAX - totals[i]) {
            printf("Сумма для ресурса %s слишком велика.\n", name);
            fclose(input); return;
        }
        totals[i] += quantity;
        valid_records++;
    }
    fclose(input);

    if (valid_records == 0) {
        printf("В input.txt нет корректных записей.\n"); return;
    }

    if (fopen_s(&output, "output.txt", "w") != 0) output = NULL;
    if (output == NULL) {
        printf("Не удалось открыть output.txt.\n"); return;
    }

    printf("\n===== СВОДНАЯ ВЕДОМОСТЬ =====\n");
    for (i = 0; i < unique_count; i++) {
        printf("%s: %d шт.\n", names[i], totals[i]);
        fprintf(output, "%s: %d шт.\n", names[i], totals[i]);
    }
    fclose(output);
    printf("Ведомость записана в output.txt.\n");
}

static void free_grid(char** grid, int rows) {
    int i;
    if (!grid) return;
    for (i = 0; i < rows; i++) free(grid[i]);
    free(grid);
}

ForestMap* forest_create(int distance) {
    int i, j, empty_count;
    ForestMap* m = (ForestMap*)malloc(sizeof(ForestMap));
    if (!m) return NULL;
    memset(m, 0, sizeof(*m));

    m->width = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    m->height = MAP_MIN_SIZE + rand() % (MAP_MAX_SIZE - MAP_MIN_SIZE + 1);
    m->distance = distance;

    m->grid = (char**)calloc((size_t)m->height, sizeof(char*));
    if (!m->grid) { free(m); return NULL; }

    for (i = 0; i < m->height; i++) {
        m->grid[i] = (char*)malloc((size_t)m->width * sizeof(char));
        if (!m->grid[i]) {
            free_grid(m->grid, m->height);
            free(m);
            return NULL;
        }
    }

    for (i = 0; i < m->height; i++) {
        for (j = 0; j < m->width; j++) {
            if (rand() % 100 < OBSTACLE_CHANCE) {
                m->grid[i][j] = (rand() % 2 == 0) ? CELL_TREE : CELL_ROCK;
            }
            else {
                m->grid[i][j] = CELL_EMPTY;
            }
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

void forest_free(ForestMap* map) {
    if (!map) return;
    free_grid(map->grid, map->height);
    free(map);
}

void forest_draw(const ForestMap* map) {
    int i, j;
    printf("\n+");
    for (j = 0; j < map->width; j++) printf("-");
    printf("+\n");

    for (i = 0; i < map->height; i++) {
        printf("|");
        for (j = 0; j < map->width; j++) {
            char c = map->grid[i][j];
            if (i == map->hero_y && j == map->hero_x) c = CELL_HERO;
            putchar(c);
        }
        printf("|\n");
    }

    printf("+");
    for (j = 0; j < map->width; j++) printf("-");
    printf("+\n");
}

void forest_place_hero_at(ForestMap* map, int px, int py) {
    int best_d = INT_MAX, bx = -1, by = -1, x, y;
    for (y = 0; y < map->height; y++) {
        for (x = 0; x < map->width; x++) {
            if (map->grid[y][x] == CELL_EMPTY || map->grid[y][x] == CELL_CAMP) {
                int d = abs(x - px) + abs(y - py);
                if (d < best_d) { best_d = d; bx = x; by = y; }
            }
        }
    }
    if (bx >= 0) { map->hero_x = bx; map->hero_y = by; }
}

int forest_move(ForestMap* map, int dx, int dy) {
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

int forest_at_camp(const ForestMap* map) {
    return map->hero_x == map->camp_x && map->hero_y == map->camp_y;
}

void adventure_run(const char* farmer_name) {
    ForestMap* map;
    int running = 1;
    char line[MAX_LINE_LEN];

    (void)farmer_name;

    map = forest_create(0);
    if (!map) {
        printf("Не удалось создать карту (нет памяти).\n");
        return;
    }

    printf("\n=== ПРИКЛЮЧЕНИЕ НАЧАЛОСЬ ===\n");
    printf("Управление: W/A/S/D - шаг, Q - домой (только на костре), M - меню\n");

    while (running) {
        char cmd;
        int  k, dx = 0, dy = 0, move_attempt = 0, result;

        forest_draw(map);
        printf("\nДальность от дома: %d. ", map->distance);
        if (forest_at_camp(map)) printf("[ВЫ НА КОСТРЕ!] ");
        printf("\nДействие: ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;
        k = 0;
        while (line[k] == ' ' || line[k] == '\t') k++;
        cmd = line[k];
        if (cmd == '\0' || cmd == '\n' || cmd == '\r') continue;

        switch (cmd) {
        case 'w': case 'W': dx = 0;  dy = -1; move_attempt = 1; break;
        case 'a': case 'A': dx = -1; dy = 0;  move_attempt = 1; break;
        case 's': case 'S': dx = 0;  dy = 1;  move_attempt = 1; break;
        case 'd': case 'D': dx = 1;  dy = 0;  move_attempt = 1; break;

        case 'q': case 'Q':
            if (forest_at_camp(map)) {
                printf("Вы возвращаетесь домой.\n");
                running = 0;
            }
            else {
                printf("Вернуться домой можно только стоя на костре '^'.\n");
            }
            break;

        case 'm': case 'M':
            printf("Вы покидаете приключение и возвращаетесь в меню.\n");
            running = 0;
            break;

        default:
            printf("Неправильный ввод.\n");
            break;
        }

        if (!move_attempt) continue;

        result = forest_move(map, dx, dy);

        if (result == MOVE_BLOCKED) {
            printf("Препятствие.\n");
        }
        else if (result == MOVE_EXIT) {
            int old_x = map->hero_x;
            int old_y = map->hero_y;
            int new_dist = map->distance + 1;

            forest_free(map);

            map = forest_create(new_dist);
            if (!map) {
                printf("Ошибка: не удалось создать новую карту!\n");
                return;
            }

            if (dx > 0)       forest_place_hero_at(map, 0, old_y);
            else if (dx < 0)  forest_place_hero_at(map, map->width - 1, old_y);
            else if (dy > 0)  forest_place_hero_at(map, old_x, 0);
            else if (dy < 0)  forest_place_hero_at(map, old_x, map->height - 1);

            printf("Вы ушли дальше от дома. Дальность: %d.\n", map->distance);
        }
    }

    forest_free(map);
}