#include <math.h>

float cos_derivative(float a, float dx) {
    return (cosf(a + dx) - cosf(a - dx)) / (2 * dx);
}

float e(int x) {
    float result = 1.0f;
    float factorial = 1.0f;
    
    for (int n = 1; n <= x; n++) {
        factorial *= n;
        result += 1.0f / factorial;
    }
    return result;
}
