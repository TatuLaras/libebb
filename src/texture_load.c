#include "texture_load.h"

#include "aseprite_texture.h"
#include <raylib.h>
#include <sys/stat.h>

// Replaces last three characters with "aseprite".
// static char *get_corresponding_aseprite_file(const char *src) {
//     size_t length = strlen(src);
//     if (length < 3)
//         return 0;
//     size_t target_length = length + 6;
//
//     char *destination = (char *)malloc(target_length);
//     memcpy(destination, src, length);
//     destination[target_length - 9] = 0;
//     strcat(destination, "aseprite");
//
//     return destination;
// }

Texture texture_load_aseprite_texture(const char *filepath) {
    ImageData image_data = aseprite_load(filepath);
    if (!image_data.base_image.data)
        return (Texture){0};

    Texture2D texture = LoadTextureFromImage(image_data.base_image);
    aseprite_image_data_free(&image_data);

    return texture;
}

void texture_load_model_texture(const char *filepath, Model *model) {
    ImageData image_data = aseprite_load(filepath);

    if (image_data.base_image.data) {
        Texture texture = LoadTextureFromImage(image_data.base_image);
        if (texture.id)
            model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }

    if (image_data.unlit_data.data) {
        Texture texture = LoadTextureFromImage(image_data.unlit_data);
        if (texture.id)
            model->materials[0].maps[MATERIAL_MAP_DIFFUSE + 1].texture =
                texture;
    }

    aseprite_image_data_free(&image_data);
}
