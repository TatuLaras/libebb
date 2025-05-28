#ifndef _LIGHTING
#define _LIGHTING

// Handling of light sources and associated vertex lighting shaders.

#include "scene.h"
#include <raylib.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#define LIGHTING_MAX_LIGHTS 16

typedef enum {
    LIGHT_NULL,
    LIGHT_POINT,
    LIGHT_DIRECTIONAL,
} LightType;

typedef struct {
    int is_disabled;
    float intensity;
    float intensity_granular;
    float intensity_cap;
    LightType type;
    Vector3 position;
    Vector3 target;
    Color color;

} LightSource;

typedef struct {
    uint32_t is_enabled_location;
    uint32_t intensity_location;
    uint32_t intensity_cap_location;
    uint32_t type_location;
    uint32_t position_location;
    uint32_t target_location;
    uint32_t color_location;
} ShaderLightSource;

typedef struct {
    Shader shader;
    ShaderLightSource shader_light_sources[LIGHTING_MAX_LIGHTS];
    uint32_t is_shading_disabled_location;
    uint32_t ambient_color_location;
} LightingShader;

typedef struct {
    LightSource lights[LIGHTING_MAX_LIGHTS];
    size_t lights_size;
    int is_deleted;
    int is_shading_disabled;
    LightingShader base_shader;
    LightingShader terrain_shader;
    Color ambient_color;

} LightingScene;

// Initializes a lighting scene.
void lighting_scene_init(Color ambient_color, const char *vert_shader,
                         const char *entity_frag_shader,
                         const char *terrain_frag_shader);

// Adds a `light` source to the lighting scene. Handle of the added light source
// will be written to `out_light_source_handle`.
int lighting_scene_add_light(LightSource light,
                             LightSourceHandle *out_light_source_handle);
// Applies the lighting scene shader to `material`.
void lighting_scene_add_material(Material *material);
// Applies the lighting scene shader to terrain material `material`.
void lighting_scene_add_terrain_material(Material *material);
// Applies the lighting scene shader to `entity`.
void lighting_scene_add_entity(Entity *entity);
// Sets whether or not lighting calculations are enabled in the lighting scene.
// If enabled is 0, the scene will be unlit.
void lighting_scene_set_enabled(uint32_t enabled);
// Get a pointer to a LightSource by its handle `light_handle`.
LightSource *lighting_scene_get_light(LightSourceHandle light_handle);

// Updates shader light calculation data for the whole scene.
void lighting_shader_data_update(void);
// Updates shader light calculation data for a light source.
void lighting_light_update(LightSourceHandle light_handle, Vector3 offset);

void lighting_scene_set_ambient_color(Color color);
Color lighting_scene_get_ambient_color(void);

#endif
