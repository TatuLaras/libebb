#include "common.h"

#include <math.h>
#include <raylib.h>
#include <stdint.h>

uint64_t max(uint64_t a, uint64_t b) {
    if (a > b)
        return a;
    return b;
}

uint64_t min(uint64_t a, uint64_t b) {
    if (a < b)
        return a;
    return b;
}

float maxf(float a, float b) {
    if (a > b)
        return a;
    return b;
}

float minf(float a, float b) {
    if (a < b)
        return a;
    return b;
}

float quantize(float value, float interval) {
    return roundf(value / interval) * interval;
}

Vector3 matrix_get_position(Matrix transform) {
    return (Vector3){transform.m12, transform.m13, transform.m14};
}
