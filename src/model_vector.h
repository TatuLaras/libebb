#ifndef _MODEL_VECTOR
#define _MODEL_VECTOR

#include "vec.h"
#include <raylib.h>

typedef struct {
    Model model;
} ModelData;

VEC_DECLARE(ModelData, ModelVector, modelvec)

#endif
