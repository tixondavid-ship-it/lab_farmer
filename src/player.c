#define _CRT_SECURE_NO_WARNINGS
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "player.h"

Player* player_create(const char* name) {
    static const int default_inv[INVENTORY_SIZE] = { 3, 1, 2, 4, 3, 5, 1, 6, 0, 1 };
    Player* p = (Player*)malloc(sizeof(Player));
    if (!p) return NULL;
    memset(p, 0, sizeof(*p));

    strncpy(p->name, name, MAX_FARMER_NAME_LEN - 1);
    p->name[MAX_FARMER_NAME_LEN - 1] = '\0';

    p->max_hp = BASE_HP;
    p->max_stamina = BASE_STAMINA;
    p->hp = p->max_hp;
    p->stamina = p->max_stamina;
    p->gold = START_GOLD;
    p->x = p->y = 0;
    p->relic = NULL;
    memcpy(p->inventory, default_inv, sizeof(default_inv));

    return p;
}

void player_free(Player** ptr) {
    if (!ptr || !*ptr) return;
    Player* p = *ptr;
    if (p->relic) {
        relic_free(&p->relic);
    }
    debug_free_message(p);
    free(p);
    *ptr = NULL;
}

Relic* relic_create(enum RelicEffect effect) {
    Relic* r = (Relic*)malloc(sizeof(Relic));
    if (!r) return NULL;
    r->effect = effect;
    switch (effect) {
    case RELIC_GREED_RING:
        snprintf(r->name, MAX_NAME_LEN, "Кольцо жадности");
        break;
    case RELIC_SWIFT_BOOTS:
        snprintf(r->name, MAX_NAME_LEN, "Сапоги-скороходы");
        break;
    case RELIC_TROLL_STONE:
        snprintf(r->name, MAX_NAME_LEN, "Камень тролля");
        break;
    default:
        snprintf(r->name, MAX_NAME_LEN, "Неизвестная реликвия");
        break;
    }
    return r;
}

Relic* relic_random(void) {
    int max_e = RELIC_COUNT - 1;
    int pick = 1 + rand() % max_e;
    return relic_create((enum RelicEffect)pick);
}

void relic_free(Relic** ptr) {
    if (!ptr || !*ptr) return;
    debug_free_message(*ptr);
    free(*ptr);
    *ptr = NULL;
}

void player_print_card(const Player* player, const GameState* gs) {
    int i;
    if (!player) { printf("Персонаж не создан.\n"); return; }
    printf("\n===== КАРТОЧКА ПЕРСОНАЖА =====\n");
    printf("Имя: %s\n", player->name);
    printf("HP: %d/%d\n", player->hp, player->max_hp);
    printf("Выносливость: %d/%d\n", player->stamina, player->max_stamina);
    printf("Золото: %d\n", player->gold);
    if (player->relic)
        printf("Реликвия: %s\n", player->relic->name);
    else
        printf("Реликвия: нет\n");
    printf("Время: день %d, %02d:00\n", gs->day, gs->hour);
    printf("Инвентарь:\n");
    for (i = 0; i < INVENTORY_SIZE; i++)
        printf("  Слот %d: [%d] - %s\n",
            i, player->inventory[i], gs->item_names[player->inventory[i]]);
}

void player_print_inventory(const Player* player, const GameState* gs) {
    int i;
    if (!player) { printf("Персонаж не создан.\n"); return; }
    printf("\n===== ИНВЕНТАРЬ =====\n");
    for (i = 0; i < INVENTORY_SIZE; i++)
        printf("Слот %d: [%d] - %s\n",
            i, player->inventory[i], gs->item_names[player->inventory[i]]);
}

void player_put_item(Player* player, const GameState* gs) {
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
    if (item_id < 0 || item_id >= MAX_ITEMS) {
        printf("ID должен быть от 0 до 9.\n"); return;
    }
    player->inventory[slot] = item_id;
    printf("В слот %d помещён предмет: %s.\n", slot, gs->item_names[item_id]);
}

void player_discard_item(Player* player) {
    int slot;
    if (!read_int("Введите номер слота от 0 до 9: ", &slot)) {
        printf("Некорректный номер слота.\n"); return;
    }
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Такого слота не существует.\n"); return;
    }
    player->inventory[slot] = 0;
    printf("Слот %d очищен.\n", slot);
}

void player_review_resources(const Player* player) {
    int id, count = 0, i;
    if (!read_int("Введите ID предмета для поиска: ", &id)) {
        printf("Некорректный ID.\n"); return;
    }
    if (id < 0 || id >= MAX_ITEMS) {
        printf("ID должен быть от 0 до 9.\n"); return;
    }
    printf("Позиции предмета: ");
    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i] == id) { printf("%d ", i); count++; }
    }
    printf("\nКоличество предметов с ID %d: %d\n", id, count);
    if (count == 0) printf("Такой предмет не найден.\n");
}

void player_search_by_name(const Player* player, const GameState* gs) {
    char name[MAX_NAME_LEN];
    int  id = -1, count = 0, i;
    if (!read_line("Введите название предмета: ", name, sizeof(name))) {
        printf("Не удалось прочитать название.\n"); return;
    }
    for (i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(name, gs->item_names[i]) == 0) { id = i; break; }
    }
    if (id == -1) { printf("Такого названия нет в каталоге.\n"); return; }
    printf("Предмет найден в слотах: ");
    for (i = 0; i < INVENTORY_SIZE; i++)
        if (player->inventory[i] == id) { printf("%d ", i); count++; }
    if (count == 0) printf("не найден");
    printf("\n");
}

void player_rest(Player* player, GameState* gs) {
    gs->day += 1;
    gs->hour = SLEEP_HOUR;
    player->hp = player->max_hp;
    player->stamina = player->max_stamina;
    printf("Вы отдохнули. День %d, %02d:00. HP и выносливость восстановлены.\n",
        gs->day, gs->hour);
}

void player_write_diary(const Player* player, const GameState* gs) {
    FILE* file = NULL;
    int i;

    if (!player) { printf("Персонаж не создан.\n"); return; }

    file = fopen(DIARY_FILE, "a");
    if (file == NULL) {
        printf("Не удалось открыть %s для записи.\n", DIARY_FILE);
        return;
    }
    fprintf(file, "===== Дневник фермера =====\n");
    fprintf(file, "Фермер: %s\n", player->name);
    fprintf(file, "День: %d, время: %02d:00\n", gs->day, gs->hour);
    fprintf(file, "HP: %d/%d, Stamina: %d/%d, Gold: %d\n",
        player->hp, player->max_hp,
        player->stamina, player->max_stamina, player->gold);
    fprintf(file, "Инвентарь:\n");
    for (i = 0; i < INVENTORY_SIZE; i++)
        fprintf(file, "Слот %d: %s\n", i, gs->item_names[player->inventory[i]]);
    fprintf(file, "\n");
    fclose(file);
    printf("Запись добавлена в %s.\n", DIARY_FILE);
}