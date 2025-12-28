#include <math.h>

float cos_derivative(float a, float dx) {
    return (cosf(a + dx) - cosf(a)) / dx;
}

float e(int x) {
    if (x == 0) return 1.0f;
    float base = 1.0f + 1.0f / (float)x;
    float result = 1.0f;
    for (int i = 0; i < x; i++) {
        result *= base;
    }
    return result;
}
