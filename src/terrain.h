#ifndef _TERRAIN
#define _TERRAIN

// A module that handles storing and managing terrain data as a heightmap, along
// with mesh generation from that data.

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdint.h>

#define TERRAIN_GRID_DENSITY 1.0
#define TERRAIN_TEXTURE_TILE_DENSITY 2.0
#define TERRAIN_HANDLE_RADIUS 11.0

typedef struct {
    Vector2 top_left_world_pos;
    uint32_t width;
    float *heights;
    size_t size;
} Terrain;

typedef struct {
    float *heights;
    size_t size;
    size_t width;
} TerrainHeights;

extern Terrain terrain;

void terrain_init(uint32_t width);
// Sets a value in the heightmap at coordinates `x`, `y` to `height`.
void terrain_set_height(uint32_t x, uint32_t y, float height);
// Get bounding box for terrain.
BoundingBox terrain_get_bounds(void);

// Resize terrain to `width`.
void terrain_resize(uint32_t width);

// Generates a GPU mesh from terrain height data.
Mesh terrain_generate_mesh(void);

#endif
