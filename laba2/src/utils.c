#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void timer_start(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

void timer_stop(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
}

double timer_elapsed_ms(Timer* timer) {
    double start_ms = timer->start.tv_sec * 1000.0 + timer->start.tv_nsec / 1000000.0;
    double end_ms = timer->end.tv_sec * 1000.0 + timer->end.tv_nsec / 1000000.0;
    return end_ms - start_ms;
}

void generate_random_matrix(const char* filename, int rows, int cols) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to create matrix file");
        return;
    }

    fprintf(file, "%d %d\n", rows, cols);
    srand(time(NULL));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fprintf(file, "%.6f ", (float)rand() / RAND_MAX * 100.0f);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Generated matrix %dx%d to %s\n", rows, cols, filename);
}

void generate_kernel(const char* filename, int size) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to create kernel file");
        return;
    }

    fprintf(file, "%d %d\n", size, size);

    float value = 1.0f / (size * size);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%.6f ", value);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Generated %dx%d kernel to %s\n", size, size, filename);
}
