#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define MAX_LINE_LEN          256
#define MAX_NAME_LEN          64
#define MAX_FARMER_NAME_LEN   33
#define MAX_ITEMS             10

#define START_DAY             1
#define START_HOUR            8
#define SLEEP_HOUR            8
#define NIGHT_HOUR            2
#define HOURS_PER_DAY         24

#define DIARY_FILE            "diary.txt"
#define ITEMS_FILE            "items.txt"
#define DELIVERY_IN_FILE      "input.txt"
#define DELIVERY_OUT_FILE     "output.txt"

typedef struct {
    int  day;
    int  hour;
    char item_names[MAX_ITEMS][MAX_NAME_LEN];
} GameState;

int  read_int(const char* prompt, int* value);
int  read_line(const char* prompt, char* buffer, size_t buffer_size);
int  read_yes_no(const char* prompt);
void trim_newline(char* text);
void debug_free_message(void* ptr);

void load_item_names(GameState* gs);
void show_time(const GameState* gs);

void summarize_deliveries(void);

#endif // UTILS_H