#include <stdio.h>
#define inventory_size 10

int main(void) {
    int current_day = 1;
    int current_hour = 8;

    int inventory[inventory_size] = { 3, 1, 2, 4, 3, 5, 1, 6, 0, 1 };

    int action;

    do {
        printf("\n===== МЕНЮ =====\n");
        printf("[0] Выход\n");
        printf("[1] Посмотреть на часы\n");
        printf("[2] Промотать время (поработать)\n");
        printf("[3] Посмотреть инвентарь\n");
        printf("[4] Положить предмет в слот\n");
        printf("[5] Выбросить предмет\n");
        printf("[6] Ревизия ресурсов\n");
        printf("Выберите пункт: ");

        scanf("%d", &action);

        switch (action) {
        case 0:
            printf("Пока-пока!\n");
            break;

        case 1:
            printf(
                "Текущее время: %d день, %02d:00\n",
                current_day,
                current_hour
            );
            break;

        case 2: {
            int hours;

            printf("Сколько часов работать? ");
            scanf("%d", &hours);

            if (hours < 0) {
                printf("Нельзя.\n");
                break;
            }

            current_hour += hours;

            current_day += current_hour / 24;
            current_hour %= 24;

            printf(
                "Текущее время: день %d, %02d:00\n",
                current_day,
                current_hour
            );
            break;
        }

        case 3:
            printf("\n===== ИНВЕНТАРЬ =====\n");

            for (int i = 0; i < 10; i++) {
                printf("Слот %d: [%d] ", i, inventory[i]);

                switch (inventory[i]) {
                case 0:
                    printf("(Пусто)");
                    break;
                case 1:
                    printf("(Дерево)");
                    break;
                case 2:
                    printf("(Камень)");
                    break;
                case 3:
                    printf("(Семена)");
                    break;
                case 4:
                    printf("(Хлеб)");
                    break;
                case 5:
                    printf("(Уголь)");
                    break;
                case 6:
                    printf("(Саженец)");
                    break;
                case 7:
                    printf("(Удобрение)");
                    break;
                case 8:
                    printf("(Ткань)");
                    break;
                case 9:
                    printf("(Факел)");
                    break;
                default:
                    printf("(Неизвестный предмет)");
                }

                printf("\n");
            }

            break;

        case 4: {
            int slot;
            int item_id;

            printf("Введите индекс слота от 0 до 9: ");
            scanf("%d", &slot);

            if (slot < 0 || slot >= inventory_size) {
                printf("Такого слота не существует.\n");
                break;
            }

            printf("Введите ID предмета: ");
            scanf("%d", &item_id);

            if (item_id < 0 || item_id > 9) {
                printf("ID от 0 до 9.\n");
                break;
            }

            inventory[slot] = item_id;

            printf("Предмет помещён в слот %d.\n", slot);
        }
              break;

        case 5: {
            int slot;

            printf("Введите номер слота от 0 до 9: ");
            scanf("%d", &slot);

            if (slot < 0 || slot >= inventory_size) {
                printf("Такого слота не существует.\n");
                break;
            }

            inventory[slot] = 0;

            printf("Слот %d очищен.\n", slot);
            break;

        case 6: {
            int searched_id;
            int count = 0;

            printf("Введите ID предмета для поиска: ");
            scanf("%d", &searched_id);

            printf("Позиции предмета: ");

            for (int i = 0; i < inventory_size; i++) {
                if (inventory[i] == searched_id) {
                    printf("%d ", i);
                    count++;
                }
            }

            printf("\nКоличество предметов с ID %d: %d\n",
                searched_id,
                count);

            if (count == 0) {
                printf("Такой предмет не найден.\n");
            }
        }
              break;
        }
        default:
            printf("Ошибка ввода\n");
            break;
        }

        } while (action != 0);

        return 0;
    }
