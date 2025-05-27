#include "systems.h"

VEC_IMPLEMENT(VoidFunction, FunctionPointerVector, funcvec)

static FunctionPointerVector update_systems = {0};

void systems_init(void) {
    update_systems = funcvec_init();
}

void systems_add_update(VoidFunction function_ptr) {
    funcvec_append(&update_systems, function_ptr);
}

void systems_run_update(void) {
    size_t i = 0;
    VoidFunction *function_ptr = 0;
    while ((function_ptr = funcvec_get(&update_systems, i++))) {
        (*function_ptr)();
    }
}

void systems_free(void) {
    funcvec_free(&update_systems);
}
