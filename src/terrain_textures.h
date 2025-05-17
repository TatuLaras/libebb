#ifndef _TERRAIN_TEXTURES
#define _TERRAIN_TEXTURES

// Functions for handling assets.

#include "handles.h"
#include "string_vector.h"
#include <raylib.h>
#include <stddef.h>

#ifndef MAX_TERRAIN_TEXTURES
#define MAX_TERRAIN_TEXTURES 10
#endif

extern StringVector terrain_texture_list;

// Gets the list of all terrain textures from `texture_directory` to the terrain
// texture list.
void terrain_textures_fetch_all(const char *texture_directory);
char *terrain_textures_get_name(TerrainTextureHandle handle);
// Gets handle of terrain texture `name`, returns 1 if there is no such texture.
int terrain_textures_get_handle(const char *name,
                                TerrainTextureHandle *out_handle);

// Sets a texture `slot` of `terrain_shader` to aseprite texture loaded from
// `texture_filepath`.
void terrain_textures_load_shader_texture(Shader *terrain_shader, uint8_t slot,
                                          const char *texture_filepath);

#endif
