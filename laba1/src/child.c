#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	char buf[4096];
	ssize_t bytes;

	pid_t pid = getpid();

	// NOTE: `O_WRONLY` only enables file for writing
	// NOTE: `O_CREAT` creates the requested file if absent
	// NOTE: `O_TRUNC` empties the file prior to opening
	// NOTE: `O_APPEND` subsequent writes are being appended instead of overwritten
	int32_t file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0600);
	if (file == -1) {
		const char msg[] = "error: failed to open requested file\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		exit(EXIT_FAILURE);
	}

	while (bytes = read(STDIN_FILENO, buf, sizeof(buf))) {
		if (bytes < 0) {
			const char msg[] = "error: failed to read from stdin\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			exit(EXIT_FAILURE);
		}

		// NOTE: Transform data
        double sum = 0.0, tmp;
        char* ptr = buf;
        char* end;

        while (ptr < &(buf[bytes - 1])) {
            while (isspace(*ptr)) {
                ++ptr;
            }
            if (ptr == &(buf[bytes - 1])) {
                break;
            }

            tmp = strtod(ptr, &end);

            if (!isspace(*end) && *end != '\n') {
                const char msg[] = "error: uexpected symbol\n";
                write(STDERR_FILENO, msg, sizeof(msg));
                exit(EXIT_FAILURE);
            }

            sum += tmp;
            ptr = end;
        }

        char out[512];
        int32_t out_len= snprintf(out, sizeof(out), "%.4f\n", sum);
        if (out_len < 5) {
            const char msg[] = "error: failed to write to file\n";
            write(STDERR_FILENO, msg, sizeof(msg));
            exit(EXIT_FAILURE);
        }
        out[out_len] = '\n';

		{
			// NOTE: Log to file
			int32_t written = write(file, out, out_len);
			if (written == 0) {
				const char msg[] = "error: failed to write to file\n";
				write(STDERR_FILENO, msg, sizeof(msg));
				exit(EXIT_FAILURE);
			}
		}
	}

	// NOTE: Write terminator to the end file
	if (bytes == 0) {
		const char term = '\0';
		write(file, &term, sizeof(term));
	}

	close(file);
}
