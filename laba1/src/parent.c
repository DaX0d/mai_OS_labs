#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static char SERVER_PROGRAM_NAME[] = "child";

int main(int argc, char* argv[]) {
    // Проверка правильности ввода команды
    if (argc != 2) {
		char msg[1024];
		uint32_t len = snprintf(msg, sizeof(msg) - 1, "usage: %s filename\n", argv[0]);
		write(STDERR_FILENO, msg, len);
		exit(EXIT_SUCCESS);
	}

    // Находим путь к директории
    char progpath[1024];
	{
		ssize_t len = readlink("/proc/self/exe", progpath,
		                       sizeof(progpath) - 1);
		if (len == -1) {
			const char msg[] = "error: failed to read full program path\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}
		while (progpath[len] != '/')
			--len;
		progpath[len] = '\0';
	}

    // Открываем пайп между клиентом и сервером
    int client_to_server[2];
	if (pipe(client_to_server) == -1) {
		const char msg[] = "error: failed to create pipe\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

    // Создаем новый процесс
    const pid_t child = fork();

	switch (child) {
        case -1: // NOTE: Kernel fails to create another process
            const char msg[] = "error: failed to spawn new process\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        break;

        case 0:
            {
                pid_t pid = getpid();

                char msg[64];
                const int32_t length = snprintf(msg, sizeof(msg),
                    "%d: I'm a child\n", pid);
                write(STDOUT_FILENO, msg, length);
            }

            close(client_to_server[1]);

            dup2(client_to_server[0], STDIN_FILENO);
            close(client_to_server[0]);

            {
                char path[2048];
                snprintf(path, sizeof(path) - 1, "%s/%s", progpath, SERVER_PROGRAM_NAME);

                char* const args[] = {SERVER_PROGRAM_NAME, argv[1], NULL};

                int32_t status = execv(path, args);

                if (status == -1) {
                    const char msg[] = "error: failed to exec into new exectuable image\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    exit(EXIT_FAILURE);
                }
            }
        break;

        default:
            {
                pid_t pid = getpid(); // NOTE: Get parent PID

                char msg[64];
                const int32_t length = snprintf(msg, sizeof(msg),
                    "%d: I'm a parent, my child has PID %d\n", pid, child);
                write(STDOUT_FILENO, msg, length);
            }

            close(client_to_server[0]);

            char buf[4096];
            ssize_t bytes;

            while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
                if (bytes < 0) {
                    const char msg[] = "error: failed to read from stdin\n";
                    write(STDERR_FILENO, msg, sizeof(msg));
                    exit(EXIT_FAILURE);
                } else if (buf[0] == '\n') {
                    // NOTE: When Enter is pressed with no input, then exit client
                    break;
                }

                write(client_to_server[1], buf, bytes);

                // bytes = read(server_to_client[0], buf, sizeof(bytes));
                // write(STDOUT_FILENO, buf, bytes);
            }

            close(client_to_server[1]);

            wait(NULL);
        break;
	}
}
