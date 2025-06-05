#ifndef _TEXTURE_LOAD
#define _TEXTURE_LOAD

#include <raylib.h>
#include <stdint.h>

// Loads an aseprite file corresponding to the model file `filepath` as a
// texture to `model` if one exists.
void texture_load_model_texture(const char *filepath, Model *model);
// Load .aseprite file as a GPU texture.
Texture texture_load_aseprite_texture(const char *filepath);

#endif
