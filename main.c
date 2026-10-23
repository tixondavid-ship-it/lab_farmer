#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <locale.h>

#define INVENTORY_SIZE 10
#define MAX_ITEMS 10
#define MAX_NAME_LEN 64
#define MAX_FARMER_NAME_LEN 33
#define MAX_LINE_LEN 256

#define MIN_ITEM_ID 0
#define MAX_ITEM_ID (MAX_ITEMS - 1)

static const char* DEFAULT_ITEM_NAMES[MAX_ITEMS] = {
    "Пусто",
    "Дерево",
    "Камень",
    "Семена",
    "Хлеб",
    "Уголь",
    "Саженец",
    "Удобрение",
    "Ткань",
    "Факел"
};

int read_int(const char* prompt, int* value);
int read_line(const char* prompt, char* buffer, size_t buffer_size);
void trim_newline(char* text);

void load_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]);
void copy_default_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]);

void print_menu(const char* farmer_name);
void print_inventory(
    const int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
);
void show_time(int current_day, int current_hour);
void work(int* current_day, int* current_hour);

void put_item(
    int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
);
void discard_item(int inventory[INVENTORY_SIZE]);
void review_resources(const int inventory[INVENTORY_SIZE]);
void search_item_by_name(
    const int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
);

void write_diary(
    const char* farmer_name,
    int current_day,
    int current_hour,
    const int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
);
void summarize_deliveries(void);

int main(void) {
    
    setlocale(LC_ALL, "");

    char farmer_name[MAX_FARMER_NAME_LEN];
    char item_names[MAX_ITEMS][MAX_NAME_LEN];

    int inventory[INVENTORY_SIZE] = { 3, 1, 2, 4, 3, 5, 1, 6, 0, 1 };
    int current_day = 1;
    int current_hour = 8;
    int action;

    printf("Введите имя фермера: ");

    if (!read_line("", farmer_name, sizeof(farmer_name)) ||
        farmer_name[0] == '\0') {
        strcpy_s(farmer_name, sizeof(farmer_name), "Фермер");
    }

    load_item_names(item_names);

    do {
        print_menu(farmer_name);

        if (!read_int("Выберите пункт: ", &action)) {
            printf("Введите целое число от 0 до 9.\n");
            continue;
        }

        switch (action) {
        case 0:
            printf("Пока-пока, %s!\n", farmer_name);
            break;

        case 1:
            show_time(current_day, current_hour);
            break;

        case 2:
            work(&current_day, &current_hour);
            break;

        case 3:
            print_inventory(inventory, item_names);
            break;

        case 4:
            put_item(inventory, item_names);
            break;

        case 5:
            discard_item(inventory);
            break;

        case 6:
            review_resources(inventory);
            break;

        case 7:
            search_item_by_name(inventory, item_names);
            break;

        case 8:
            write_diary(
                farmer_name,
                current_day,
                current_hour,
                inventory,
                item_names
            );
            break;

        case 9:
            summarize_deliveries();
            break;

        default:
            printf("Выберите пункт от 0 до 9.\n");
            break;
        }
    } while (action != 0);

    return 0;
}

int read_int(const char* prompt, int* value) {
    char line[MAX_LINE_LEN];
    char extra;
    int result;

    printf("%s", prompt);

    if (!fgets(line, sizeof(line), stdin)) {
        return 0;
    }

    result = sscanf_s(line, " %d %c", value, &extra);
    return result == 1;
}

int read_line(const char* prompt, char* buffer, size_t buffer_size) {
    size_t length;

    printf("%s", prompt);

    if (!fgets(buffer, (int)buffer_size, stdin)) {
        return 0;
    }

    length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    }
    else {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF) {
            /* Удаляем остаток слишком длинной строки. */
        }
    }

    trim_newline(buffer);
    return 1;
}

void trim_newline(char* text) {
    size_t length = strlen(text);

    while (length > 0 &&
        (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        text[--length] = '\0';
    }
}
void copy_default_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    int i;

    for (i = 0; i < MAX_ITEMS; i++) {
        snprintf(item_names[i], MAX_NAME_LEN, "%s", DEFAULT_ITEM_NAMES[i]);
    }
}

void load_item_names(char item_names[MAX_ITEMS][MAX_NAME_LEN]) {
    FILE* file = NULL;
    int id;
    char name[MAX_NAME_LEN];
    int loaded_count = 0;

    copy_default_item_names(item_names);

    if (fopen_s(&file, "items.txt", "r") != 0) {
        file = NULL;
    }

    if (file == NULL) {
        printf("Предупреждение: items.txt не найден. "
            "Используются стандартные названия.\n");
        return;
    }

    while (fscanf_s(file, "%d %63s",
        &id, name, (unsigned)sizeof(name)) == 2) {
        if (id >= MIN_ITEM_ID && id <= MAX_ITEM_ID) {
            snprintf(item_names[id], MAX_NAME_LEN, "%s", name);
            loaded_count++;
        }
    }

    fclose(file);

    if (loaded_count == 0) {
        printf("Предупреждение: в items.txt не найдено корректных записей. "
            "Используются стандартные названия.\n");
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
}

void show_time(int current_day, int current_hour) {
    printf("Текущее время: %d день, %02d:00\n", current_day, current_hour);
}

void work(int* current_day, int* current_hour) {
    int hours;

    if (!read_int("Сколько часов работать? ", &hours)) {
        printf("Введите целое число.\n");
        return;
    }

    if (hours < 0) {
        printf("Количество часов не может быть отрицательным.\n");
        return;
    }

    if (hours > INT_MAX - *current_hour) {
        printf("Слишком большое количество часов.\n");
        return;
    }

    *current_hour += hours;
    *current_day += *current_hour / 24;
    *current_hour %= 24;

    show_time(*current_day, *current_hour);
}

void print_inventory(
    const int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
) {
    int i;

    printf("\n===== ИНВЕНТАРЬ =====\n");

    for (i = 0; i < INVENTORY_SIZE; i++) {
        printf("Слот %d: [%d] - %s\n",
            i, inventory[i], item_names[inventory[i]]);
    }
}

void put_item(
    int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
) {
    int slot;
    int item_id;

    if (!read_int("Введите индекс слота от 0 до 9: ", &slot)) {
        printf("Некорректный номер слота.\n");
        return;
    }

    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Такого слота не существует.\n");
        return;
    }

    if (!read_int("Введите ID предмета от 0 до 9: ", &item_id)) {
        printf("Некорректный ID.\n");
        return;
    }

    if (item_id < MIN_ITEM_ID || item_id > MAX_ITEM_ID) {
        printf("ID должен быть от 0 до 9.\n");
        return;
    }

    inventory[slot] = item_id;
    printf("В слот %d помещён предмет: %s.\n",
        slot, item_names[item_id]);
}

void discard_item(int inventory[INVENTORY_SIZE]) {
    int slot;

    if (!read_int("Введите номер слота от 0 до 9: ", &slot)) {
        printf("Некорректный номер слота.\n");
        return;
    }

    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Такого слота не существует.\n");
        return;
    }

    inventory[slot] = 0;
    printf("Слот %d очищен.\n", slot);
}

void review_resources(const int inventory[INVENTORY_SIZE]) {
    int searched_id;
    int count = 0;
    int i;

    if (!read_int("Введите ID предмета для поиска: ", &searched_id)) {
        printf("Некорректный ID.\n");
        return;
    }

    if (searched_id < MIN_ITEM_ID || searched_id > MAX_ITEM_ID) {
        printf("ID должен быть от 0 до 9.\n");
        return;
    }

    printf("Позиции предмета: ");

    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == searched_id) {
            printf("%d ", i);
            count++;
        }
    }

    printf("\nКоличество предметов с ID %d: %d\n", searched_id, count);

    if (count == 0) {
        printf("Такой предмет не найден.\n");
    }
}
void search_item_by_name(
    const int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
) {
    char searched_name[MAX_NAME_LEN];
    int item_id = -1;
    int count = 0;
    int i;

    if (!read_line("Введите название предмета: ",
        searched_name, sizeof(searched_name))) {
        printf("Не удалось прочитать название.\n");
        return;
    }

    for (i = 0; i < MAX_ITEMS; i++) {
        if (strcmp(searched_name, item_names[i]) == 0) {
            item_id = i;
            break;
        }
    }

    if (item_id == -1) {
        printf("Такого названия нет в каталоге.\n");
        return;
    }

    printf("Предмет найден в слотах: ");

    for (i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == item_id) {
            printf("%d ", i);
            count++;
        }
    }

    if (count == 0) {
        printf("не найден");
    }

    printf("\n");
}

void write_diary(
    const char* farmer_name,
    int current_day,
    int current_hour,
    const int inventory[INVENTORY_SIZE],
    const char item_names[MAX_ITEMS][MAX_NAME_LEN]
) {
    FILE* file = NULL;
    int i;

    if (fopen_s(&file, "diary.txt", "a") != 0) {
        file = NULL;
    }

    if (file == NULL) {
        printf("Не удалось открыть diary.txt для записи.\n");
        return;
    }

    fprintf(file, "===== Дневник фермера =====\n");
    fprintf(file, "Фермер: %s\n", farmer_name);
    fprintf(file, "День: %d, время: %02d:00\n", current_day, current_hour);
    fprintf(file, "Инвентарь:\n");

    for (i = 0; i < INVENTORY_SIZE; i++) {
        fprintf(file, "Слот %d: %s\n", i, item_names[inventory[i]]);
    }

    fprintf(file, "\n");
    fclose(file);

    printf("Запись добавлена в diary.txt.\n");
}
void summarize_deliveries(void) {
    FILE* input = NULL;
    FILE* output = NULL;

    char names[MAX_ITEMS][MAX_NAME_LEN];
    int totals[MAX_ITEMS] = { 0 };
    int unique_count = 0;
    char name[MAX_NAME_LEN];
    int quantity;
    int i;
    int valid_records = 0;

    if (fopen_s(&input, "input.txt", "r") != 0) {
        input = NULL;
    }

    if (input == NULL) {
        printf("Не удалось открыть input.txt. "
            "Добавьте строки вида: Дерево 10\n");
        return;
    }

    while (fscanf_s(input, "%63s %d",
        name, (unsigned)sizeof(name), &quantity) == 2) {
        if (quantity < 0) {
            printf("Пропущена запись с отрицательным количеством: "
                "%s %d\n", name, quantity);
            continue;
        }

        for (i = 0; i < unique_count; i++) {
            if (strcmp(names[i], name) == 0) {
                break;
            }
        }

        if (i == unique_count) {
            if (unique_count >= MAX_ITEMS) {
                printf("Слишком много разных ресурсов: максимум %d.\n",
                    MAX_ITEMS);
                fclose(input);
                return;
            }

            snprintf(names[unique_count], MAX_NAME_LEN, "%s", name);
            totals[unique_count] = 0;
            unique_count++;
        }

        if (quantity > INT_MAX - totals[i]) {
            printf("Сумма количества для ресурса %s слишком велика.\n",
                name);
            fclose(input);
            return;
        }

        totals[i] += quantity;
        valid_records++;
    }

    fclose(input);

    if (valid_records == 0) {
        printf("В input.txt нет корректных записей. "
            "Используйте формат: Название Количество.\n");
        return;
    }

    if (fopen_s(&output, "output.txt", "w") != 0) {
        output = NULL;
    }

    if (output == NULL) {
        printf("Не удалось открыть output.txt для записи.\n");
        return;
    }

    printf("\n===== СВОДНАЯ ВЕДОМОСТЬ =====\n");

    for (i = 0; i < unique_count; i++) {
        printf("%s: %d шт.\n", names[i], totals[i]);
        fprintf(output, "%s: %d шт.\n", names[i], totals[i]);
    }

    fclose(output);
    printf("Ведомость записана в output.txt.\n");
}