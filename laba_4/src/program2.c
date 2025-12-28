#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

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

    void* library_handle = NULL;
    float (*cos_derivative_ptr)(float, float) = NULL;
    float (*e_ptr)(int) = NULL;

    int current_lib = 1;

    write_string("Программа с динамической загрузкой библиотек\n");
    write_string("Команды:\n");
    write_string("  0 - переключить реализацию\n");
    write_string("  1 a dx - производная cos в точке a с шагом dx\n");
    write_string("  2 x - вычисление e с точностью x\n");
    write_string("  q - выход\n");

    library_handle = dlopen("./libfunc1.so", RTLD_LAZY);
    if (!library_handle) {
        write_string("Ошибка загрузки библиотеки: ");
        write_string(dlerror());
        write_string("\n");
        return 1;
    }

    cos_derivative_ptr = dlsym(library_handle, "cos_derivative");
    e_ptr = dlsym(library_handle, "e");

    if (!cos_derivative_ptr || !e_ptr) {
        write_string("Ошибка получения функций: ");
        write_string(dlerror());
        write_string("\n");
        dlclose(library_handle);
        return 1;
    }

    while (1) {
        write_string("\nТекущая реализация: ");
        write_int(current_lib);
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

        if (strcmp(tokens[0], "0") == 0) {
            dlclose(library_handle);

            if (current_lib == 1) {
                library_handle = dlopen("./libfunc2.so", RTLD_LAZY);
                current_lib = 2;
                write_string("Переключено на реализацию 2\n");
            } else {
                library_handle = dlopen("./libfunc1.so", RTLD_LAZY);
                current_lib = 1;
                write_string("Переключено на реализацию 1\n");
            }

            if (!library_handle) {
                write_string("Ошибка загрузки библиотеки: ");
                write_string(dlerror());
                write_string("\n");
                return 1;
            }

            cos_derivative_ptr = dlsym(library_handle, "cos_derivative");
            e_ptr = dlsym(library_handle, "e");

            if (!cos_derivative_ptr || !e_ptr) {
                write_string("Ошибка получения функций: ");
                write_string(dlerror());
                write_string("\n");
                dlclose(library_handle);
                return 1;
            }
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

            float result = cos_derivative_ptr(a, dx);
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

            float result = e_ptr(x);
            write_string("Результат: ");
            write_float(result);
            write_string("\n");
        }
        else if (strcmp(tokens[0], "q") == 0) {
            write_string("Выход\n");
            break;
        }
        else {
            write_string("Неизвестная команда\n");
        }
    }

    if (library_handle) {
        dlclose(library_handle);
    }

    return 0;
}
