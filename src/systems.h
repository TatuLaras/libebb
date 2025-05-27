#ifndef _SYSTEMS
#define _SYSTEMS

/*
A module for registering functions to be called on start and on each frame, aka.
systems of an Entity Component System.
*/

#include "vec.h"

typedef void (*VoidFunction)(void);
VEC_DECLARE(VoidFunction, FunctionPointerVector, funcvec)

// Initializes the module, call this before any of the other functions.
void systems_init(void);

// Adds an update function to be run each time systems_run_update is called.
void systems_add_update(VoidFunction function_ptr);
// Calls all functions registered with systems_add_update.
void systems_run_update(void);

// Frees memory associated with this module.
void systems_free(void);

#endif
