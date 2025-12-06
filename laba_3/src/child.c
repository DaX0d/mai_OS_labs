#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_NUMBERS 100
#define BUFFER_SIZE 256

typedef struct {
    float numbers[MAX_NUMBERS];
    int count;
    bool done;
    bool ready;
} SharedData;

void write_string(int fd, const char* str) {
    write(fd, str, strlen(str));
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        char msg[128];
        int len = snprintf(msg, sizeof(msg), 
                          "Использование: %s <файл_результата> <shm_name> <sem_parent> <sem_child> <parent_pid>\n", 
                          argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_FAILURE);
    }

    const char* result_filename = argv[1];
    const char* shm_name = argv[2];
    const char* sem_parent_name = argv[3];
    const char* sem_child_name = argv[4];
    pid_t parent_pid = atoi(argv[5]);

    char msg[128];
    int len = snprintf(msg, sizeof(msg), 
                      "Дочерний процесс (PID: %d). Родительский PID: %d\n", 
                      getpid(), parent_pid);
    write(STDOUT_FILENO, msg, len);

    // Открытие файла для записи результатов
    int result_fd = open(result_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (result_fd == -1) {
        write_string(STDERR_FILENO, "Ошибка: не удалось открыть файл для записи\n");
        exit(EXIT_FAILURE);
    }

    len = snprintf(msg, sizeof(msg), 
                  "Результат будет записан в файл: %s\n", result_filename);
    write(STDOUT_FILENO, msg, len);

    // Открытие разделяемой памяти
    int shm_fd = shm_open(shm_name, O_RDWR, 0);
    if (shm_fd == -1) {
        write_string(STDERR_FILENO, "Ошибка: не удалось открыть shared memory\n");
        exit(EXIT_FAILURE);
    }

    SharedData* data = mmap(NULL, sizeof(SharedData), 
                           PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        write_string(STDERR_FILENO, "Ошибка: не удалось отобразить shared memory\n");
        exit(EXIT_FAILURE);
    }

    // Открытие семафоров
    sem_t *sem_parent = sem_open(sem_parent_name, 0);
    sem_t *sem_child = sem_open(sem_child_name, 0);
    
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        write_string(STDERR_FILENO, "Ошибка: не удалось открыть семафоры\n");
        exit(EXIT_FAILURE);
    }

    // Основной цикл обработки
    while (1) {
        // Ждем данных от родителя
        sem_wait(sem_child);

        if (data->done) {
            break;
        }

        if (data->ready && data->count >= 0) {
            // Вычисляем сумму чисел из текущей строки
            float line_sum = 0.0f;

            for (int i = 0; i < data->count; i++) {
                line_sum += data->numbers[i];
            }

            // Записываем сумму строки в файл
            if (data->count > 0) {
                char output[64];
                int output_len = snprintf(output, sizeof(output), "%.4f\n", line_sum);
                write(result_fd, output, output_len);
            }

            // Сбрасываем флаг готовности
            data->ready = false;

            // Сигнализируем родителю, что данные обработаны
            sem_post(sem_parent);
        }
    }

    close(result_fd);

    // Закрытие ресурсов
    munmap(data, sizeof(SharedData));
    close(shm_fd);

    sem_close(sem_parent);
    sem_close(sem_child);

    write_string(STDOUT_FILENO, "Дочерний процесс завершен. Суммы строк записаны в файл.\n");

    return 0;
}
