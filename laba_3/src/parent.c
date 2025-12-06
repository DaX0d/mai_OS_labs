#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <ctype.h>

#define SHM_SIZE 1024
#define MAX_NUMBERS 100
#define BUFFER_SIZE 256

// Структура для разделяемой памяти
typedef struct {
    float numbers[MAX_NUMBERS];  // массив чисел
    int count;                    // количество чисел
    bool done;                    // флаг завершения
    bool ready;                   // флаг готовности данных
} SharedData;

void write_string(int fd, const char* str) {
    write(fd, str, strlen(str));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        char msg[128];
        int len = snprintf(msg, sizeof(msg), "Использование: %s <имя_файла_для_результата>\n", argv[0]);
        write(STDERR_FILENO, msg, len);
        exit(EXIT_FAILURE);
    }

    // Генерация уникальных имен на основе PID
    pid_t pid = getpid();
    char shm_name[256];
    char sem_parent_name[256];
    char sem_child_name[256];

    snprintf(shm_name, sizeof(shm_name), "/lab_shm_%d", pid);
    snprintf(sem_parent_name, sizeof(sem_parent_name), "/lab_sem_parent_%d", pid);
    snprintf(sem_child_name, sizeof(sem_child_name), "/lab_sem_child_%d", pid);

    // Создание разделяемой памяти
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        write_string(STDERR_FILENO, "Ошибка: не удалось создать shared memory\n");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        write_string(STDERR_FILENO, "Ошибка: не удалось изменить размер shared memory\n");
        exit(EXIT_FAILURE);
    }

    SharedData* data = mmap(NULL, sizeof(SharedData), 
                           PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED) {
        write_string(STDERR_FILENO, "Ошибка: не удалось отобразить shared memory\n");
        exit(EXIT_FAILURE);
    }

    // Инициализация данных
    data->count = 0;
    data->done = false;
    data->ready = false;

    // Создание семафоров
    sem_t *sem_parent = sem_open(sem_parent_name, O_CREAT, 0666, 0);
    sem_t *sem_child = sem_open(sem_child_name, O_CREAT, 0666, 0);

    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        write_string(STDERR_FILENO, "Ошибка: не удалось создать семафоры\n");
        exit(EXIT_FAILURE);
    }

    // Создание дочернего процесса
    pid_t child_pid = fork();

    if (child_pid == -1) {
        write_string(STDERR_FILENO, "Ошибка: не удалось создать дочерний процесс\n");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {  // Дочерний процесс
        // Передача имен разделяемой памяти и семафоров дочернему процессу
        char pid_str[32];
        snprintf(pid_str, sizeof(pid_str), "%d", pid);

        execl("./child", "child", argv[1], shm_name, sem_parent_name, 
              sem_child_name, pid_str, NULL);

        // Если exec не сработал
        write_string(STDERR_FILENO, "Ошибка: не удалось запустить дочернюю программу\n");
        exit(EXIT_FAILURE);
    } 
    else {  // Родительский процесс
        char msg[128];
        int len = snprintf(msg, sizeof(msg), 
                          "Родительский процесс (PID: %d). Вводите строки с числами через пробел.\n", pid);
        write(STDOUT_FILENO, msg, len);
        write_string(STDOUT_FILENO, "Для завершения ввода введите пустую строку.\n");

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;

        while (1) {
            // Чтение строки из stdin
            bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

            if (bytes_read <= 0) {
                break;
            }

            buffer[bytes_read] = '\0';

            // Проверка на пустую строку (только \n)
            if (bytes_read == 1 && buffer[0] == '\n') {
                break;
            }

            // Сбрасываем счетчик чисел
            data->count = 0;

            // Парсинг чисел из строки
            char* ptr = buffer;
            char* end;

            while (*ptr != '\0' && data->count < MAX_NUMBERS) {
                // Пропускаем пробелы
                while (isspace(*ptr)) {
                    ptr++;
                }

                if (*ptr == '\0' || *ptr == '\n') {
                    break;
                }

                // Парсим число
                float num = strtof(ptr, &end);

                if (ptr == end) {  // Не удалось распарсить число
                    break;
                }

                // Сохраняем число
                data->numbers[data->count] = num;
                data->count++;
                ptr = end;
            }

            // Если в строке были числа
            if (data->count > 0) {
                // Сигнализируем дочернему процессу, что данные готовы
                data->ready = true;
                sem_post(sem_child);

                // Ждем, пока дочерний процесс обработает данные
                sem_wait(sem_parent);
            }
        }

        // Сигнализируем о завершении
        data->done = true;
        sem_post(sem_child);

        write_string(STDOUT_FILENO, "Ожидание завершения дочернего процесса...\n");
        wait(NULL);

        // Закрытие и удаление ресурсов
        munmap(data, sizeof(SharedData));
        close(shm_fd);
        shm_unlink(shm_name);

        sem_close(sem_parent);
        sem_close(sem_child);
        sem_unlink(sem_parent_name);
        sem_unlink(sem_child_name);

        write_string(STDOUT_FILENO, "Родительский процесс завершен.\n");
    }

    return 0;
}
