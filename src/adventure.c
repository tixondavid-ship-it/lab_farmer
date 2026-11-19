#define _CRT_SECURE_NO_WARNINGS
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "adventure.h"
#include "map.h"
#include <stdio.h>
#include <stdlib.h>

static int advance_time(GameState* gs, int hours) {
    gs->hour += hours;
    while (gs->hour >= HOURS_PER_DAY) {
        gs->hour -= HOURS_PER_DAY;
        gs->day++;
    }
    return gs->hour == NIGHT_HOUR;
}

static int player_gold_gain_per_floor(const Player* player, int floor) {
    int mult = ADV_GOLD_PER_FLOOR;
    if (player->relic && player->relic->effect == RELIC_GREED_RING)
        mult *= ADV_GREED_GOLD_MULT;
    return mult * floor;
}

static int player_step_cost(const Player* player) {
    int cost = ADV_STEP_STAMINA;
    if (player->relic && player->relic->effect == RELIC_GREED_RING)
        cost = ADV_GREED_STEP_STAMINA;
    return cost;
}

static int player_floor_cost(const Player* player) {
    if (player->relic && player->relic->effect == RELIC_SWIFT_BOOTS)
        return ADV_SWIFT_FLOOR_STAMINA;
    return ADV_FLOOR_STAMINA;
}

static void offer_relic(Player* player) {
    Relic* new_relic = relic_random();
    int   choice = 1;

    if (!new_relic) return;

    printf("\nВы находите реликвию: %s!\n", new_relic->name);
    printf("[1] Надеть (старая реликвия пропадёт)\n");
    printf("[2] Продать за %d золота\n", ADV_RELIC_SELL_PRICE);

    if (!read_int("Выбор: ", &choice) || (choice != 1 && choice != 2))
        choice = 2;

    if (choice == 1) {
        if (player->relic) {
            printf("Старая реликвия (%s) пропала.\n", player->relic->name);
            relic_free(&player->relic);
        }
        player->relic = new_relic;
        printf("Надета реликвия: %s\n", player->relic->name);
    }
    else {
        player->gold += ADV_RELIC_SELL_PRICE;
        printf("Реликвия продана за %d золота.\n", ADV_RELIC_SELL_PRICE);
        relic_free(&new_relic);
    }
}

static void handle_floor_transition(Player* player, GameState* gs,
    int* floor, int* prev_x, int* prev_y,
    int dx, int dy, int night, Map** p_map) {
    int night_hit;
    Map* old = *p_map;
    int old_x = old->hero_x, old_y = old->hero_y;

    (void)prev_x; (void)prev_y;

    player->stamina -= player_floor_cost(player);
    if (player->stamina < 0) player->stamina = 0;

    *floor += 1;
    night_hit = advance_time(gs, ADV_FLOOR_TIME);

    {
        int gain = player_gold_gain_per_floor(player, *floor);
        player->gold += gain;
        printf("Новый этаж: %d. Награда: %d золота.\n", *floor, gain);
    }

    if (rand() % 100 < ADV_RELIC_CHANCE)
        offer_relic(player);

    map_free(p_map);
    *p_map = map_create(*floor, night);
    if (!*p_map) {
        printf("Ошибка: не удалось создать новую карту!\n");
        return;
    }

    if (dx > 0)       map_place_hero_at(*p_map, 0, old_y);
    else if (dx < 0)  map_place_hero_at(*p_map, (*p_map)->width - 1, old_y);
    else if (dy > 0)  map_place_hero_at(*p_map, old_x, 0);
    else if (dy < 0)  map_place_hero_at(*p_map, old_x, (*p_map)->height - 1);

    if (night_hit)
        printf("Время достигает 02:00...\n");
}

void adventure_run(Player* player, GameState* gs, int is_night) {
    Map* map;
    int  running = 1;
    int  emergency = 0;
    int  floor = 1;

    if (!player) { printf("Сначала создайте персонажа.\n"); return; }
    if (player->stamina <= 0) { printf("Нет сил для приключения.\n"); return; }
    if (gs->hour == NIGHT_HOUR) { printf("Сейчас 02:00, нужно спать.\n"); return; }

    map = map_create(floor, is_night);
    if (!map) { printf("Не удалось создать карту.\n"); return; }

    printf("\n=== ПРИКЛЮЧЕНИЕ НАЧАЛОСЬ ===\n");
    printf("W/A/S/D - шаг, R - крик (ночь), Q - выйти на костре, H - сбежать (штраф 50%% золота)\n");
        printf("Ночной лес. R подсветит окрестности.\n");

    while (running) {
        char line[MAX_LINE_LEN];
        char cmd;
        int  dx = 0, dy = 0, move_attempt = 0;

        map_draw(map);

        printf("\nHP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00\n",
            player->hp, player->max_hp,
            player->stamina, player->max_stamina,
            player->gold, floor, gs->hour);
        if (player->relic)
            printf("Реликвия: %s\n", player->relic->name);
        if (map_at_camp(map)) printf("[ВЫ НА КОСТРЕ - можно выйти через Q]\n");
        if (map->is_night && map->reveal_steps_left > 0)
            printf("[Подсвечено ещё %d шагов]\n", map->reveal_steps_left);

        printf("Действие: ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        cmd = line[0];

        switch (cmd) {
        case 'w': case 'W': dx = 0;  dy = -1; move_attempt = 1; break;
        case 'a': case 'A': dx = -1; dy = 0;  move_attempt = 1; break;
        case 's': case 'S': dx = 0;  dy = 1;  move_attempt = 1; break;
        case 'd': case 'D': dx = 1;  dy = 0;  move_attempt = 1; break;

        case 'r': case 'R': map_shout(map); break;

        case 'q': case 'Q':
            if (map_at_camp(map)) {
                printf("Вы благополучно возвращаетесь домой.\n");
                running = 0;
            }
            else {
                printf("Выйти можно только стоя на костре '^'.\n");
            }
            break;

        case 'h': case 'H':
        {
            int lost_gold = player->gold / 2;
            player->gold -= lost_gold;
            printf("\nВы в панике бежите из леса, петляя между деревьями!\n");
            printf("Спасатели находят вас на опушке. За эвакуацию они забирают половину золота.\n");
            printf("Потеряно: %d золота. Остаток: %d\n", lost_gold, player->gold);
            running = 0;
        }
        break;

        default:
            printf("Неизвестная команда.\n");
            break;
        }

        if (!move_attempt) continue;

        {
            int result = map_move(map, dx, dy);
            if (result == MOVE_BLOCKED) {
                printf("Препятствие.\n");
            }
            else if (result == MOVE_OK) {
                int cost = player_step_cost(player);
                player->stamina -= cost;
                if (player->stamina < 0) player->stamina = 0;

                if (player->relic && player->relic->effect == RELIC_TROLL_STONE) {
                    player->hp += ADV_TROLL_HP_PER_STEP;
                    if (player->hp > player->max_hp) player->hp = player->max_hp;
                }

                if (map->is_night && map->reveal_steps_left > 0) {
                    int i;
                    map->reveal_steps_left--;
                    if (map->reveal_steps_left == 0) {
                        for (i = 0; i < map->height; i++)
                            memset(map->revealed[i], 0,
                                (size_t)map->width);
                        printf("Темнота снова сгущается...\n");
                    }
                }
            }
            else if (result == MOVE_EXIT) {
                handle_floor_transition(player, gs, &floor,
                    &map->hero_x, &map->hero_y, dx, dy, is_night, &map);
                if (!map) { running = 0; break; }
                continue;
            }
        }

        if (player->stamina <= 0) {
            printf("\nВы падаете без сил!\n");
            emergency = 1;
            running = 0;
        }
        if (gs->hour == NIGHT_HOUR) {
            printf("\nНаступает 02:00, вы теряете сознание...\n");
            emergency = 1;
            running = 0;
        }
    }

    if (emergency) {
        player->gold /= ADV_EMERGENCY_GOLD_DIV;
        printf("Неизвестно как вы оказались дома, потеря половину золота.\n");
        printf("Осталось золота: %d\n", player->gold);
    }

    if (map) map_free(&map);
}