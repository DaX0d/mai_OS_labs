#include "convolution.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

Matrix* create_matrix(int rows, int cols) {
    Matrix* m = (Matrix*)malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->data = (float**)malloc(rows * sizeof(float*));
    for (int i = 0; i < rows; i++) {
        m->data[i] = (float*)calloc(cols, sizeof(float));
    }
    return m;
}

void free_matrix(Matrix* m) {
    if (!m) return;
    for (int i = 0; i < m->rows; i++) {
        free(m->data[i]);
    }
    free(m->data);
    free(m);
}

Matrix* read_matrix_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    int rows, cols;
    fscanf(file, "%d %d", &rows, &cols);

    Matrix* m = create_matrix(rows, cols);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%f", &m->data[i][j]);
        }
    }

    fclose(file);
    return m;
}

void write_matrix_to_file(const char* filename, Matrix* m) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    fprintf(file, "%d %d\n", m->rows, m->cols);

    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            fprintf(file, "%.6f ", m->data[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void print_matrix(Matrix* m) {
    if (!m) return;
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            printf("%.2f ", m->data[i][j]);
        }
        printf("\n");
    }
}

void apply_convolution_partial(Matrix* input, Matrix* kernel, Matrix* output, int start_row, int end_row) {
    int k_half = kernel->rows / 2;
    int k_size = kernel->rows;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < input->cols; j++) {
            float sum = 0.0f;

            for (int ki = 0; ki < k_size; ki++) {
                for (int kj = 0; kj < k_size; kj++) {
                    int input_i = i + ki - k_half;
                    int input_j = j + kj - k_half;

                    if (input_i >= 0 && input_i < input->rows && 
                        input_j >= 0 && input_j < input->cols) {
                        sum += input->data[input_i][input_j] * kernel->data[ki][kj];
                    }
                }
            }

            output->data[i][j] = sum;
        }
    }
}

void* convolution_thread_func(void* arg) {
    ConvolutionTask* task = (ConvolutionTask*)arg;
    Matrix* temp1 = task->input;
    Matrix* temp2 = task->output;

    for (int iter = 0; iter < task->iterations; iter++) {
        apply_convolution_partial(temp1, task->kernel, temp2, 
                                 task->start_row, task->end_row);

        pthread_barrier_wait(task->barrier);

        Matrix* temp = temp1;
        temp1 = temp2;
        temp2 = temp;
    }

    return NULL;
}

Matrix* apply_convolution_sequential(Matrix* input, Matrix* kernel, int iterations) {
    Matrix* result = create_matrix(input->rows, input->cols);
    Matrix* buffer1 = create_matrix(input->rows, input->cols);
    Matrix* buffer2 = create_matrix(input->rows, input->cols);

    for (int i = 0; i < input->rows; i++) {
        memcpy(buffer1->data[i], input->data[i], input->cols * sizeof(float));
    }

    for (int iter = 0; iter < iterations; iter++) {
        apply_convolution_partial(buffer1, kernel, buffer2, 0, input->rows);
        
        Matrix* temp = buffer1;
        buffer1 = buffer2;
        buffer2 = temp;
    }

    for (int i = 0; i < input->rows; i++) {
        memcpy(result->data[i], buffer1->data[i], input->cols * sizeof(float));
    }

    free_matrix(buffer1);
    free_matrix(buffer2);

    return result;
}

Matrix* apply_convolution_parallel(Matrix* input, Matrix* kernel, int iterations, int num_threads) {
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    ConvolutionTask* tasks = (ConvolutionTask*)malloc(num_threads * sizeof(ConvolutionTask));

    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, num_threads);

    Matrix* buffer1 = create_matrix(input->rows, input->cols);
    Matrix* buffer2 = create_matrix(input->rows, input->cols);

    for (int i = 0; i < input->rows; i++) {
        memcpy(buffer1->data[i], input->data[i], input->cols * sizeof(float));
    }

    int rows_per_thread = input->rows / num_threads;
    int extra_rows = input->rows % num_threads;

    int current_row = 0;
    for (int i = 0; i < num_threads; i++) {
        tasks[i].input = buffer1;
        tasks[i].output = buffer2;
        tasks[i].kernel = kernel;
        tasks[i].iterations = iterations;
        tasks[i].barrier = &barrier;

        tasks[i].start_row = current_row;
        tasks[i].end_row = current_row + rows_per_thread + (i < extra_rows ? 1 : 0);
        current_row = tasks[i].end_row;

        pthread_create(&threads[i], NULL, convolution_thread_func, &tasks[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    Matrix* result = create_matrix(input->rows, input->cols);
    for (int i = 0; i < input->rows; i++) {
        memcpy(result->data[i], buffer1->data[i], input->cols * sizeof(float));
    }

    pthread_barrier_destroy(&barrier);
    free_matrix(buffer1);
    free_matrix(buffer2);
    free(threads);
    free(tasks);

    return result;
}
