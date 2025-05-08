#include "model_files.h"

#include "aseprite_texture.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint64_t file_last_modified(const char *filepath) {
    struct stat attr;
    if (stat(filepath, &attr))
        return 0;
    return attr.st_mtim.tv_sec;
}

// Replaces last three characters with "aseprite".
static char *path_get_corresponding_texture_file(const char *src) {
    size_t length = strlen(src);
    if (length < 3)
        return 0;
    size_t target_length = length + 6;

    char *destination = (char *)malloc(target_length);
    memcpy(destination, src, length);
    destination[target_length - 9] = 0;
    strcat(destination, "aseprite");

    return destination;
}

void load_aseprite_texture(const char *filepath, Model *model) {
    Image *image = aseprite_load_as_image(filepath);
    if (!image)
        return;

    Texture2D texture = LoadTextureFromImage(*image);
    UnloadImage(*image);
    free(image);
    if (!texture.id)
        return;

    model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

void try_load_corresponding_texture(const char *filepath, Model *model) {
    char *texture_file = path_get_corresponding_texture_file(filepath);
    load_aseprite_texture(texture_file, model);
    free(texture_file);
}
