#ifndef _HANDLES
#define _HANDLES

#include <stddef.h>
#include <stdint.h>

typedef uint16_t LightSourceHandle;
typedef size_t EntityHandle;
typedef size_t ModelHandle;
typedef size_t AssetHandle;
typedef size_t SkyboxHandle;
typedef size_t TerrainTextureHandle;

enum {
    LOAD_KIND_MODEL,
    LOAD_KIND_TEXTURE,
    LOAD_KIND_TERRAIN_TEXTURE,
    LOAD_KIND_SKYBOX,
};

#endif
