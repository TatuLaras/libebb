#ifndef _COMMON
#define _COMMON

#include <raylib.h>
#include <stdint.h>

#define ARRAY_LENGTH(x) ((sizeof x) / sizeof(*x))
#define NAME_MAX_LENGTH 128

uint64_t max(uint64_t a, uint64_t b);
uint64_t min(uint64_t a, uint64_t b);
float maxf(float a, float b);
float minf(float a, float b);

// Quantize `value` to `interval`.
float quantize(float value, float interval);
// Gets a position from a `transform` matrix.
Vector3 matrix_get_position(Matrix transform);

#endif
