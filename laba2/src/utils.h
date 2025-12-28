#pragma once

#include <time.h>

typedef struct {
    struct timespec start;
    struct timespec end;
} Timer;

void timer_start(Timer* timer);
void timer_stop(Timer* timer);
double timer_elapsed_ms(Timer* timer);

void generate_random_matrix(const char* filename, int rows, int cols);
void generate_kernel(const char* filename, int size);
