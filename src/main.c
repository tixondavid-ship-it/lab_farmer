#define _CRT_SECURE_NO_WARNINGS
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "utils.h"
#include "player.h"
#include "adventure.h"

#ifdef _WIN32
#include <windows.h>
#endif

static void print_main_menu(const Player* player) {
    printf("\n===== ФЕРМА =====\n");
    if (!player) printf("[!] Персонаж не создан\n");
    else         printf("Персонаж: %s | HP: %d/%d | St: %d/%d | Gold: %d\n",
        player->name, player->hp, player->max_hp,
        player->stamina, player->max_stamina, player->gold);
    printf("[0]  Выход\n");
    printf("[1]  Создать / пересоздать персонажа\n");
    printf("[2]  Карточка персонажа\n");
    printf("[3]  Управление временем\n");
    printf("[4]  Посмотреть инвентарь\n");
    printf("[5]  Положить предмет в слот\n");
    printf("[6]  Выбросить предмет\n");
    printf("[7]  Ревизия ресурсов по ID\n");
    printf("[8]  Поиск предмета по названию\n");
    printf("[9]  Записать состояние в дневник\n");
    printf("[10] Подсчитать ресурсы из input.txt\n");
    printf("[11] Отправиться в приключение\n");
}

static void time_wait(GameState* gs) {
    int max_wait, hours;
    if (gs->hour == NIGHT_HOUR) {
        printf("Сейчас 02:00, нужно спать.\n");
        return;
    }
    if (gs->hour < NIGHT_HOUR) max_wait = NIGHT_HOUR - gs->hour;
    else                       max_wait = HOURS_PER_DAY - gs->hour + NIGHT_HOUR;

    printf("Можно подождать не больше %d часов (сейчас %02d:00).\n",
        max_wait, gs->hour);
    if (!read_int("Сколько часов ждать? ", &hours)) {
        printf("Некорректный ввод.\n"); return;
    }
    if (hours < 1 || hours > max_wait) {
        printf("Можно ждать от 1 до %d часов.\n", max_wait);
        return;
    }
    gs->hour += hours;
    while (gs->hour >= HOURS_PER_DAY) { gs->hour -= HOURS_PER_DAY; gs->day++; }
    show_time(gs);
}

static void run_time_submenu(Player* player, GameState* gs) {
    int choice;
    do {
        printf("\n=== УПРАВЛЕНИЕ ВРЕМЕНЕМ ===\n");
        printf("[0] Назад\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Подождать\n");
        printf("[3] Лечь спать (восстановление)\n");
        if (!read_int("Выберите: ", &choice)) continue;
        switch (choice) {
        case 0: break;
        case 1: show_time(gs); break;
        case 2: time_wait(gs); break;
        case 3:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_rest(player, gs);
            break;
        default: printf("Нет такого пункта.\n");
        }
    } while (choice != 0);
}

static Player* create_or_recreate_player(Player* old) {
    char name[MAX_FARMER_NAME_LEN];
    Player* p;

    if (old != NULL) {
        if (!read_yes_no("Персонаж уже существует. Пересоздать? (y/n): ")) {
            printf("Отмена.\n");
            return old;
        }
        player_free(&old);
    }

    if (!read_line("Введите имя фермера: ", name, sizeof(name))
        || name[0] == '\0') {
        snprintf(name, sizeof(name), "Фермер");
    }

    p = player_create(name);
    if (p) printf("Персонаж %s создан.\n", p->name);
    else   printf("Не удалось выделить память.\n");
    return p;
}

static void run_adventure(Player* player, GameState* gs) {
    int mode = 1;
    if (!player) { printf("Сначала создайте персонажа.\n"); return; }
    if (player->stamina <= 0) { printf("Нет сил для приключения.\n"); return; }
    if (gs->hour == NIGHT_HOUR) { printf("Сейчас 02:00, нужно спать.\n"); return; }

    if (!read_int("Какой лес? [1] Дневной [2] Ночной: ", &mode)
        || (mode != 1 && mode != 2))
        mode = 1;
    adventure_run(player, gs, mode == 2);
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    srand((unsigned)time(NULL));

    GameState gs;
    gs.day = START_DAY;
    gs.hour = START_HOUR;
    load_item_names(&gs);

    Player* player = NULL;
    int action;

    do {
        print_main_menu(player);
        if (!read_int("Выберите пункт: ", &action)) {
            printf("Введите целое число.\n");
            continue;
        }

        switch (action) {
        case 0:
            printf("Пока-пока!\n");
            break;

        case 1:
            player = create_or_recreate_player(player);
            break;

        case 2:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_print_card(player, &gs);
            break;

        case 3:
            run_time_submenu(player, &gs);
            break;

        case 4:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_print_inventory(player, &gs);
            break;

        case 5:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_put_item(player, &gs);
            break;

        case 6:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_discard_item(player);
            break;

        case 7:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_review_resources(player);
            break;

        case 8:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_search_by_name(player, &gs);
            break;

        case 9:
            if (!player) { printf("Сначала создайте персонажа.\n"); break; }
            player_write_diary(player, &gs);
            break;

        case 10:
            summarize_deliveries();
            break;

        case 11:
            run_adventure(player, &gs);
            break;

        default:
            printf("Выберите пункт от 0 до 11.\n");
        }
    } while (action != 0);

    if (player) player_free(&player);
    return 0;
}