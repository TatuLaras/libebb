#include "terrain.h"

#include <assert.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Terrain terrain;

void terrain_init(uint32_t width) {
    uint32_t halfway = width / 2;
    uint32_t corners = width + 1;
    size_t data_size = corners * corners;

    printf("%u terrain \n", halfway);
    terrain = (Terrain){
        .width = corners,
        .size = data_size,
        .heights = calloc(data_size, sizeof(float)),
        .top_left_world_pos = (Vector2){-(float)halfway, -(float)halfway},
    };
}

BoundingBox terrain_get_bounds(void) {
    Vector3 min = {
        terrain.top_left_world_pos.x,
        0,
        terrain.top_left_world_pos.y,
    };

    return (BoundingBox){
        .min = min,
        .max = Vector3Add(min, (Vector3){terrain.width, 0, terrain.width}),
    };
}

void terrain_set_height(uint32_t x, uint32_t y, float height);

void terrain_resize(uint32_t width) {
    assert(terrain.heights);
    assert(width > 0);
    int32_t corners = width + 1;

    uint32_t old_width = terrain.width;
    int64_t difference = (int64_t)corners - (int64_t)terrain.width;
    if (difference == 0)
        return;
    difference /= 2;
    printf("Resize to %u, dif: %ld\n", corners, difference);
    terrain.width = corners;
    terrain.size = corners * corners;

    uint32_t halfway = corners / 2;
    terrain.top_left_world_pos = (Vector2){-(float)halfway, -(float)halfway};

    // Allocate new resized buffer and copy height data
    float *new_buffer = malloc(terrain.size * sizeof(float));
    for (size_t i = 0; i < terrain.size; i++) {
        size_t new_x = i % corners;
        size_t new_y = i / corners;
        size_t old_x = new_x - difference;
        size_t old_y = new_y - difference;

        int inside_old_data = old_x >= 0 && old_x < old_width;
        inside_old_data = inside_old_data && old_y >= 0 && old_y < old_width;

        if (!inside_old_data) {
            new_buffer[i] = 0;
            continue;
        }

        new_buffer[i] = terrain.heights[old_y * old_width + old_x];
    }

    free(terrain.heights);
    terrain.heights = new_buffer;
}

static inline Vector3 datapoint_position(size_t i) {
    uint32_t x = i % terrain.width;
    uint32_t y = i / terrain.width;
    return (Vector3){
        terrain.top_left_world_pos.x + (float)x * TERRAIN_GRID_DENSITY,
        terrain.heights[i],
        terrain.top_left_world_pos.y + (float)y * TERRAIN_GRID_DENSITY,
    };
}

Mesh terrain_generate_mesh(void) {
    uint32_t width_cells = terrain.width - 1;
    uint64_t cell_count = width_cells * width_cells;

    Mesh mesh = {0};
    mesh.triangleCount = cell_count * 2;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = malloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = malloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = malloc(mesh.vertexCount * 3 * sizeof(float));

    uint32_t x;
    uint32_t y;
    for (size_t i = 0; i < terrain.size; i++) {
        x = i % terrain.width;
        y = i / terrain.width;

        if (x >= width_cells || y >= width_cells)
            continue;

        size_t quad = x + width_cells * y;
        size_t quad_i = quad * 18;
        uint8_t start_corner = 0;

        if (y % 2 == 0)
            start_corner = x % 2;
        else
            start_corner = 3 - (x % 2);

        Vector3 corners[4] = {
            datapoint_position(i),
            datapoint_position(i + terrain.width),
            datapoint_position(i + terrain.width + 1),
            datapoint_position(i + 1),
        };
        Vector2 corner_uvs[4] = {
            (Vector2){0, 0},
            (Vector2){0, 1},
            (Vector2){1, 1},
            (Vector2){1, 0},
        };

        // Two triangles per cell / quad
        for (uint8_t tri = 0; tri < 2; tri++) {
            size_t triangle_i = quad_i + tri * 9;
            for (uint8_t vert = 0; vert < 3; vert++) {
                uint8_t corner = (start_corner + vert + tri * 2) % 4;
                mesh.vertices[triangle_i + vert * 3 + 0] = corners[corner].x;
                mesh.vertices[triangle_i + vert * 3 + 1] = corners[corner].y;
                mesh.vertices[triangle_i + vert * 3 + 2] = corners[corner].z;

                mesh.texcoords[quad * 12 + tri * 6 + vert * 2 + 0] =
                    corner_uvs[corner].x * 0.5 + (x % 2) * 0.5;
                mesh.texcoords[quad * 12 + tri * 6 + vert * 2 + 1] =
                    corner_uvs[corner].y * 0.5 + (y % 2) * 0.5;
            }

            // Normal vector
            Vector3 *v1 = (Vector3 *)(mesh.vertices + triangle_i + 0 * 3);
            Vector3 *v2 = (Vector3 *)(mesh.vertices + triangle_i + 1 * 3);
            Vector3 *v3 = (Vector3 *)(mesh.vertices + triangle_i + 2 * 3);

            Vector3 triangle_normal = Vector3Normalize(Vector3CrossProduct(
                Vector3Subtract(*v2, *v1), Vector3Subtract(*v3, *v1)));
            for (uint8_t vert = 0; vert < 3; vert++) {
                mesh.normals[triangle_i + vert * 3 + 0] = triangle_normal.x;
                mesh.normals[triangle_i + vert * 3 + 1] = triangle_normal.y;
                mesh.normals[triangle_i + vert * 3 + 2] = triangle_normal.z;
            }
        }
    }

    // Upload mesh data from CPU (RAM) to GPU (VRAM) memory
    UploadMesh(&mesh, false);

    return mesh;
}
