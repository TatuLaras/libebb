#include "model_vector.h"
#include <assert.h>
#include <raylib.h>
#include <stdlib.h>

#define STARTING_SIZE 4
#define GROWTH_FACTOR 2

ModelVector modelvec_init(void) {
    ModelVector vec = {
        .data = malloc(STARTING_SIZE * sizeof(ModelData)),
        .data_allocated = STARTING_SIZE,
    };
    return vec;
}

ModelHandle modelvec_append(ModelVector *vec, ModelData model) {
    if (vec->data_used >= vec->data_allocated) {
        // Grow buffer
        vec->data_allocated *= GROWTH_FACTOR;
        vec->data = realloc(vec->data, vec->data_allocated * sizeof(ModelData));
        assert(vec->data);
    }

    vec->data[vec->data_used++] = model;
    return vec->data_used - 1;
}

ModelData *modelvec_get(ModelVector *vec, size_t index) {
    if (index >= vec->data_used)
        return 0;

    return vec->data + index;
}

void modelvec_free(ModelVector *vec) {
    if (vec->data) {
        free(vec->data);
        vec->data = 0;
    }
}
