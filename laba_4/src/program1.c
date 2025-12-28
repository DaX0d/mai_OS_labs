#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "functions.h"

#define BUFFER_SIZE 256

void write_string(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
}

void write_float(float value) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%.6f", value);
    write(STDOUT_FILENO, buffer, len);
}

void write_int(int value) {
    char buffer[32];
    int len = snprintf(buffer, sizeof(buffer), "%d", value);
    write(STDOUT_FILENO, buffer, len);
}

int parse_float(const char* str, float* result) {
    char* endptr;
    *result = strtof(str, &endptr);
    return (*endptr == '\0' || *endptr == '\n');
}

int parse_int(const char* str, int* result) {
    char* endptr;
    *result = strtol(str, &endptr, 10);
    return (*endptr == '\0' || *endptr == '\n');
}

int main() {
    char buffer[BUFFER_SIZE];
    char* tokens[10];

    write_string("Программа с статической линковкой\n");
    write_string("Команды:\n");
    write_string("  1 a dx - производная cos в точке a с шагом dx\n");
    write_string("  2 x - вычисление e с точностью x\n");
    write_string("  q - выход\n");

    while (1) {
        write_string("\nВведите команду: ");

        int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) continue;

        buffer[bytes_read] = '\0';

        int token_count = 0;
        char* token = strtok(buffer, " \n");
        while (token != NULL && token_count < 10) {
            tokens[token_count++] = token;
            token = strtok(NULL, " \n");
        }

        if (token_count == 0) continue;

        if (strcmp(tokens[0], "q") == 0) {
            write_string("Выход\n");
            break;
        }
        else if (strcmp(tokens[0], "1") == 0) {
            if (token_count != 3) {
                write_string("Ошибка: требуется 2 аргумента для производной\n");
                continue;
            }

            float a, dx;
            if (!parse_float(tokens[1], &a) || !parse_float(tokens[2], &dx)) {
                write_string("Ошибка: неверный формат чисел\n");
                continue;
            }

            float result = cos_derivative(a, dx);
            write_string("Результат: ");
            write_float(result);
            write_string("\n");
        }
        else if (strcmp(tokens[0], "2") == 0) {
            if (token_count != 2) {
                write_string("Ошибка: требуется 1 аргумент для вычисления e\n");
                continue;
            }

            int x;
            if (!parse_int(tokens[1], &x)) {
                write_string("Ошибка: неверный формат числа\n");
                continue;
            }

            if (x < 0) {
                write_string("Ошибка: x должен быть неотрицательным\n");
                continue;
            }

            float result = e(x);
            write_string("Результат: ");
            write_float(result);
            write_string("\n");
        }
        else {
            write_string("Неизвестная команда\n");
        }
    }

    return 0;
}
