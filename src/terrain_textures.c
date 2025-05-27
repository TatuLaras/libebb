#include "terrain_textures.h"

#include "common.h"
#include "filesystem.h"
#include "string_vector.h"
#include "terrain.h"
#include "texture_load.h"
#include <assert.h>
#include <dirent.h>
#include <raylib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

StringVector terrain_texture_list = {0};
static TerrainTextureHandle selected_textures[TERRAIN_MAX_TEXTURES];

void terrain_textures_fetch_all(const char *skybox_directory) {
    if (!terrain_texture_list.data)
        terrain_texture_list = stringvec_init();

    stringvec_truncate(&terrain_texture_list);

    get_basenames_with_suffix(skybox_directory, &terrain_texture_list,
                              ".aseprite");
}

char *terrain_textures_get_name(SkyboxHandle handle) {
    return stringvec_get(&terrain_texture_list, handle);
}

int terrain_textures_get_handle(const char *name, SkyboxHandle *out_handle) {
    int64_t index = stringvec_index_of(&terrain_texture_list, name);
    if (index < 0)
        return 1;
    if (out_handle)
        *out_handle = index;
    return 0;
}

void terrain_textures_load_into_slot(TerrainTextureHandle handle, uint8_t slot,
                                     const char *terrain_texture_directory) {
    selected_textures[slot] = handle;

    if (!terrain_texture_directory)
        return;

    char *texture_name = terrain_textures_get_name(handle);
    char path[MAX_PATH_LENGTH] = {0};
    strcpy(path, terrain_texture_directory);
    strcat(path, texture_name);
    strcat(path, ".aseprite");

    Texture texture = texture_load_aseprite_texture(path);
    terrain_bind_texture(slot, texture);
}

void terrain_textures_load_all_selected(const char *terrain_texture_directory) {
    assert(terrain_texture_directory);

    for (size_t i = 0; i < ARRAY_LENGTH(selected_textures); i++) {
        char *texture_name = terrain_textures_get_name(selected_textures[i]);
        char path[MAX_PATH_LENGTH] = {0};
        strcpy(path, terrain_texture_directory);
        strcat(path, texture_name);
        strcat(path, ".aseprite");
        Texture texture = texture_load_aseprite_texture(path);
        terrain_bind_texture(i, texture);
    }
}

TerrainTextureHandle terrain_textures_get_slot_handle(uint8_t slot) {
    if (slot >= TERRAIN_MAX_TEXTURES)
        return 0;
    return selected_textures[slot];
}
