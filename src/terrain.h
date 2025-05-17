#ifndef _TERRAIN
#define _TERRAIN

// Handling of a heightmap terrain with texture mixing.

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdint.h>

#define TERRAIN_GRID_DENSITY 1.0
#define TERRAIN_TEXTURE_TILE_DENSITY 2.0
#define TERRAIN_HANDLE_RADIUS 11.0
#define TERRAIN_MAX_TEXTURES 7

typedef struct {
    Vector2 top_left_world_pos;
    float *heights;
    uint8_t *texture_indices;
    uint32_t width;
    size_t size;
    Material material;
} Terrain;

extern Terrain terrain;

void terrain_init(uint32_t width);
void terrain_free(void);

// Sets a value in the heightmap at coordinates `x`, `y` to `height`.
void terrain_set_height(uint32_t x, uint32_t y, float height);
// Get bounding box for terrain.
BoundingBox terrain_get_bounds(void);

// Resize terrain to `width`, while maintaining the height data in the common
// portion.
void terrain_resize(uint32_t width);

// Generates a GPU mesh from terrain height data.
Mesh terrain_generate_mesh(void);

// Loads texture into a shader location to be used for texture painting with
// asset slot `slot`.
void terrain_bind_texture(uint8_t slot, Texture texture);

#endif
