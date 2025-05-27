#ifndef _ASEPRITE_TEXTURE
#define _ASEPRITE_TEXTURE

#include "raylib.h"

typedef struct {
    Image base_image;
    Image unlit_data;
} ImageData;

ImageData aseprite_load(const char *filepath);
void aseprite_image_data_free(ImageData *image_data);

#endif
