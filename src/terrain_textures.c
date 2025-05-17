#include "terrain_textures.h"

#include "filesystem.h"
#include "string_vector.h"
#include <assert.h>
#include <dirent.h>
#include <raylib.h>
#include <stddef.h>
#include <stdint.h>

StringVector terrain_texture_list = {0};

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
