#include "convolution.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>

void print_usage(const char* prog_name) {
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  -i <file>    Input matrix file\n");
    printf("  -k <file>    Kernel file\n");
    printf("  -o <file>    Output matrix file\n");
    printf("  -t <N>       Number of threads (default: 1)\n");
    printf("  -iter <K>    Number of iterations (default: 1)\n");
    printf("  -seq         Run sequential version only\n");
    printf("  -par         Run parallel version only\n");
    printf("  -gen <size>  Generate random matrix of given size\n");
    printf("  -help        Show this help\n");
}

int main(int argc, char* argv[]) {
    char* input_file = NULL;
    char* kernel_file = NULL;
    char* output_file = "output.txt";
    int num_threads = 1;
    int iterations = 1;
    int run_seq = 0;
    int run_par = 0;
    int generate_size = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-k") == 0 && i + 1 < argc) {
            kernel_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-iter") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-seq") == 0) {
            run_seq = 1;
        } else if (strcmp(argv[i], "-par") == 0) {
            run_par = 1;
        } else if (strcmp(argv[i], "-gen") == 0 && i + 1 < argc) {
            generate_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    if (generate_size > 0) {
        char matrix_file[100];
        char kernel_file[100];
        
        sprintf(matrix_file, "matrix_%dx%d.txt", generate_size, generate_size);
        sprintf(kernel_file, "kernel_3x3.txt");
        
        generate_random_matrix(matrix_file, generate_size, generate_size);
        generate_kernel(kernel_file, 3);
        
        printf("Test data generated. Run with:\n");
        printf("  %s -i %s -k %s -t 4 -iter 3\n", argv[0], matrix_file, kernel_file);
        return 0;
    }
    
    if (!input_file || !kernel_file) {
        print_usage(argv[0]);
        return 1;
    }
    
    Matrix* input = read_matrix_from_file(input_file);
    Matrix* kernel = read_matrix_from_file(kernel_file);
    
    if (!input || !kernel) {
        fprintf(stderr, "Failed to read input files\n");
        return 1;
    }
    
    printf("Matrix: %dx%d, Kernel: %dx%d, Threads: %d, Iterations: %d\n",
           input->rows, input->cols, kernel->rows, kernel->cols, num_threads, iterations);
    
    if (!run_seq && !run_par) {
        run_seq = 1;
        run_par = 1;
    }
    
    Matrix* result_seq = NULL;
    Matrix* result_par = NULL;
    Timer timer;
    double time_seq = 0, time_par = 0;
    
    if (run_seq) {
        printf("\n=== Sequential version ===\n");
        timer_start(&timer);
        result_seq = apply_convolution_sequential(input, kernel, iterations);
        timer_stop(&timer);
        time_seq = timer_elapsed_ms(&timer);
        printf("Time: %.2f ms\n", time_seq);
    }
    
    if (run_par) {
        printf("\n=== Parallel version (%d threads) ===\n", num_threads);
        timer_start(&timer);
        result_par = apply_convolution_parallel(input, kernel, iterations, num_threads);
        timer_stop(&timer);
        time_par = timer_elapsed_ms(&timer);
        printf("Time: %.2f ms\n", time_par);
        
        if (run_seq) {
            double speedup = time_seq / time_par;
            double efficiency = speedup / num_threads;
            printf("Speedup: %.2f\n", speedup);
            printf("Efficiency: %.2f\n", efficiency);
        }
    }
    
    if (result_par) {
        write_matrix_to_file(output_file, result_par);
        printf("Result written to %s\n", output_file);
    } else if (result_seq) {
        write_matrix_to_file(output_file, result_seq);
        printf("Result written to %s\n", output_file);
    }
    
    free_matrix(input);
    free_matrix(kernel);
    free_matrix(result_seq);
    free_matrix(result_par);
    
    return 0;
}
