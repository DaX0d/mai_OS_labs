#pragma once

#include <stddef.h>
#include <pthread.h>

typedef struct {
    float** data;
    int rows;
    int cols;
} Matrix;

// Структура для параметров свёртки
typedef struct {
    Matrix* input;
    Matrix* kernel;
    Matrix* output;
    int iterations;
    int start_row;
    int end_row;
    pthread_barrier_t* barrier;
} ConvolutionTask;

Matrix* create_matrix(int rows, int cols);
void free_matrix(Matrix* m);
Matrix* read_matrix_from_file(const char* filename);
void write_matrix_to_file(const char* filename, Matrix* m);
Matrix* apply_convolution_sequential(Matrix* input, Matrix* kernel, int iterations);
Matrix* apply_convolution_parallel(Matrix* input, Matrix* kernel, int iterations, int num_threads);
void print_matrix(Matrix* m);
