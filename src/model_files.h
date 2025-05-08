#ifndef _MODEL_FILES
#define _MODEL_FILES

#include <raylib.h>
#include <stdint.h>

// Loads an aseprite file corresponding to the model file `filepath` as a
// texture to `model` if one exists.
void try_load_corresponding_texture(const char *filepath, Model *model);
void load_aseprite_texture(const char *filepath, Model *model);

#endif
