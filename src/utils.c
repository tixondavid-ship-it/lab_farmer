#define _CRT_SECURE_NO_WARNINGS
#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "utils.h"

#ifdef _WIN32
#include <windows.h>
#endif

static const char* DEFAULT_ITEM_NAMES[MAX_ITEMS] = {
    "Пусто", "Дерево", "Камень", "Семена", "Хлеб",
    "Уголь", "Саженец", "Удобрение", "Ткань", "Факел"
};

int read_int(const char* prompt, int* value) {
    char line[MAX_LINE_LEN];
    char extra = 0;
    int  result;

    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) return 0;

    result = sscanf(line, " %d %c", value, &extra);
    return result == 1;
}

int read_line(const char* prompt, char* buffer, size_t buffer_size) {
    char raw[MAX_LINE_LEN];

    printf("%s", prompt);
    fflush(stdout);
    buffer[0] = '\0';

    if (!fgets(raw, sizeof(raw), stdin)) return 0;
    trim_newline(raw);

#ifdef _WIN32
    {
        UINT cp = GetConsoleCP();
        if (cp == CP_UTF8) {
            strncpy(buffer, raw, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            return 1;
        }
        else {
            wchar_t wbuf[MAX_LINE_LEN];
            int wlen = MultiByteToWideChar(cp, 0, raw, -1, wbuf,
                (int)(sizeof(wbuf) / sizeof(wbuf[0])));
            if (wlen <= 0) return 0;
            int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1,
                buffer, (int)buffer_size, NULL, NULL);
            return ulen > 0;
        }
    }
#else
    strncpy(buffer, raw, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
    return 1;
#endif
}

int read_yes_no(const char* prompt) {
    char buffer[MAX_LINE_LEN];
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    trim_newline(buffer);

    if (strcmp(buffer, "y") == 0 || strcmp(buffer, "Y") == 0 ||
        strcmp(buffer, "yes") == 0 || strcmp(buffer, "Yes") == 0 ||
        strcmp(buffer, "д") == 0 || strcmp(buffer, "Д") == 0 ||
        strcmp(buffer, "да") == 0 || strcmp(buffer, "Да") == 0)
    {
        return 1;
    }
    return 0;
}

void trim_newline(char* text) {
    size_t length = strlen(text);
    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r'))
        text[--length] = '\0';
}

void debug_free_message(void* ptr) {
    printf("Память структуры по адресу %p освобождена\n", ptr);
}

void load_item_names(GameState* gs) {
    FILE* file = NULL;
    int   id;
    char  name[MAX_NAME_LEN];
    int   i, loaded_count = 0;

    for (i = 0; i < MAX_ITEMS; i++)
        snprintf(gs->item_names[i], MAX_NAME_LEN, "%s", DEFAULT_ITEM_NAMES[i]);

    file = fopen(ITEMS_FILE, "r");
    if (file == NULL) {
        printf("Предупреждение: %s не найден. Используются стандартные названия.\n",
            ITEMS_FILE);
        return;
    }

    while (fscanf(file, "%d %63s", &id, name) == 2) {
        if (id >= 0 && id < MAX_ITEMS) {
            snprintf(gs->item_names[id], MAX_NAME_LEN, "%s", name);
            loaded_count++;
        }
    }
    fclose(file);

    if (loaded_count == 0)
        printf("Предупреждение: в %s нет корректных записей.\n", ITEMS_FILE);
}

void show_time(const GameState* gs) {
    printf("Текущее время: %d день, %02d:00\n", gs->day, gs->hour);
}

void summarize_deliveries(void) {
    FILE* input = NULL;
    FILE* output = NULL;
    char  names[MAX_ITEMS][MAX_NAME_LEN];
    int   totals[MAX_ITEMS] = { 0 };
    int   unique_count = 0;
    char  name[MAX_NAME_LEN];
    int   quantity, i, valid_records = 0;

    input = fopen(DELIVERY_IN_FILE, "r");
    if (input == NULL) {
        printf("Не удалось открыть %s.\n", DELIVERY_IN_FILE);
        return;
    }

    while (fscanf(input, "%63s %d", name, &quantity) == 2) {
        if (quantity < 0) {
            printf("Пропущена запись с отрицательным количеством: %s %d\n",
                name, quantity);
            continue;
        }
        for (i = 0; i < unique_count; i++)
            if (strcmp(names[i], name) == 0) break;
        if (i == unique_count) {
            if (unique_count >= MAX_ITEMS) {
                printf("Слишком много разных ресурсов.\n");
                fclose(input);
                return;
            }
            snprintf(names[unique_count], MAX_NAME_LEN, "%s", name);
            totals[unique_count] = 0;
            unique_count++;
        }
        if (quantity > INT_MAX - totals[i]) {
            printf("Сумма для ресурса %s слишком велика.\n", name);
            fclose(input);
            return;
        }
        totals[i] += quantity;
        valid_records++;
    }
    fclose(input);

    if (valid_records == 0) {
        printf("В %s нет корректных записей.\n", DELIVERY_IN_FILE);
        return;
    }

    output = fopen(DELIVERY_OUT_FILE, "w");
    if (output == NULL) {
        printf("Не удалось открыть %s.\n", DELIVERY_OUT_FILE);
        return;
    }

    printf("\n===== СВОДНАЯ ВЕДОМОСТЬ =====\n");
    for (i = 0; i < unique_count; i++) {
        printf("%s: %d шт.\n", names[i], totals[i]);
        fprintf(output, "%s: %d шт.\n", names[i], totals[i]);
    }
    fclose(output);
    printf("Ведомость записана в %s.\n", DELIVERY_OUT_FILE);
}