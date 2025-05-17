#include "lighting.h"

#include "common.h"
#include "handles.h"
#include "model_vector.h"
#include "scene.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STARTING_SIZE 4
#define GROWTH_FACTOR 2

LightingScene lighting_scene = {0};

static inline LightingShader shader_init(char *vertex, char *fragment) {
    LightingShader lighting_shader = {0};
    lighting_shader.shader = LoadShader(vertex, fragment);

    lighting_shader.ambient_color_location =
        GetShaderLocation(lighting_shader.shader, "ambient");
    lighting_shader.is_shading_disabled_location =
        GetShaderLocation(lighting_shader.shader, "shadingDisabled");

    for (size_t i = 0; i < LIGHTING_MAX_LIGHTS; i++) {
        lighting_shader.shader_light_sources[i].color_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].color", i));
        lighting_shader.shader_light_sources[i].is_enabled_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].enabled", i));
        lighting_shader.shader_light_sources[i].intensity_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].intensity", i));
        lighting_shader.shader_light_sources[i].intensity_cap_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].intensity_cap", i));
        lighting_shader.shader_light_sources[i].type_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].type", i));
        lighting_shader.shader_light_sources[i].position_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].position", i));
        lighting_shader.shader_light_sources[i].target_location =
            GetShaderLocation(lighting_shader.shader,
                              TextFormat("lights[%i].target", i));
    }

    return lighting_shader;
}

void lighting_scene_init(Color ambient_color) {
    lighting_scene.ambient_color = ambient_color;

    //  TODO: Just give these as parameters
    char shader_path[MAX_PATH_LENGTH + 1] = __FILE__;
    strip_filename(shader_path, MAX_PATH_LENGTH);
    strcat(shader_path, "../resources/shaders/vertex_lighting.vert");

    char terrain_shader_path[MAX_PATH_LENGTH + 1] = __FILE__;
    strip_filename(terrain_shader_path, MAX_PATH_LENGTH);
    strcat(terrain_shader_path, "../resources/shaders/terrain.frag");

    lighting_scene.base_shader = shader_init(shader_path, 0);
    lighting_scene.terrain_shader =
        shader_init(shader_path, terrain_shader_path);

    return;
}

int lighting_scene_add_light(LightSource light,
                             LightSourceHandle *out_light_source_handle) {
    if (lighting_scene.lights_size >= LIGHTING_MAX_LIGHTS)
        return 1;

    uint32_t i = lighting_scene.lights_size;

    // Connect shader locations

    if (out_light_source_handle)
        *out_light_source_handle = lighting_scene.lights_size;

    lighting_scene.lights[lighting_scene.lights_size++] = light;

    update_shader_data();

    return 0;
}

void lighting_scene_add_material(Material *material) {
    material->shader = lighting_scene.base_shader.shader;
}

void lighting_scene_add_terrain_material(Material *material) {
    material->shader = lighting_scene.terrain_shader.shader;
}

void lighting_scene_add_entity(Entity *entity) {
    ModelData *model_data = scene_entity_get_model(entity);
    assert(model_data->model.meshCount);
    lighting_scene_add_material(&model_data->model.materials[0]);
}

LightSource *lighting_scene_get_light(LightSourceHandle light_handle) {
    if (light_handle >= lighting_scene.lights_size)
        return 0;
    return lighting_scene.lights + light_handle;
}

void lighting_scene_set_enabled(uint32_t enabled) {
    lighting_scene.is_shading_disabled = !enabled;
    update_shader_data();
}

static inline int
lighting_shader_update_light_source(LightingShader *lighting_shader,
                                    LightSourceHandle light_handle,
                                    Vector3 offset) {
    if (light_handle >= lighting_scene.lights_size)
        return 1;

    LightSource *light = lighting_scene.lights + light_handle;
    if (light->type == LIGHT_NULL)
        return 1;

    int enabled = !light->is_disabled;
    ShaderLightSource *source_locs =
        lighting_shader->shader_light_sources + light_handle;

    SetShaderValue(lighting_shader->shader, source_locs->is_enabled_location,
                   &enabled, SHADER_UNIFORM_INT);

    float intensity = light->intensity + light->intensity_granular;
    SetShaderValue(lighting_shader->shader, source_locs->intensity_location,
                   &intensity, SHADER_UNIFORM_FLOAT);

    SetShaderValue(lighting_shader->shader, source_locs->intensity_cap_location,
                   &light->intensity_cap, SHADER_UNIFORM_FLOAT);
    SetShaderValue(lighting_shader->shader, source_locs->type_location,
                   &light->type, SHADER_UNIFORM_INT);

    Vector3 light_pos = Vector3Add(light->position, offset);

    float position[3] = {light_pos.x, light_pos.y, light_pos.z};
    SetShaderValue(lighting_shader->shader, source_locs->position_location,
                   position, SHADER_UNIFORM_VEC3);

    float target[3] = {light->target.x, light->target.y, light->target.z};
    SetShaderValue(lighting_shader->shader, source_locs->target_location,
                   target, SHADER_UNIFORM_VEC3);

    float color[4] = {
        (float)light->color.r / (float)255, (float)light->color.g / (float)255,
        (float)light->color.b / (float)255, (float)light->color.a / (float)255};
    SetShaderValue(lighting_shader->shader, source_locs->color_location, color,
                   SHADER_UNIFORM_VEC4);

    return 0;
}

static inline void lighting_shader_update(LightingShader *lighting_shader) {
    float ambient_color[4] = {
        (float)lighting_scene.ambient_color.r / (float)255,
        (float)lighting_scene.ambient_color.g / (float)255,
        (float)lighting_scene.ambient_color.b / (float)255,
        (float)lighting_scene.ambient_color.a / (float)255};
    SetShaderValue(lighting_shader->shader,
                   lighting_shader->ambient_color_location, &ambient_color,
                   SHADER_UNIFORM_VEC4);

    SetShaderValue(lighting_shader->shader,
                   lighting_shader->is_shading_disabled_location,
                   &lighting_scene.is_shading_disabled, SHADER_UNIFORM_INT);

    for (size_t i = 0; i < LIGHTING_MAX_LIGHTS; i++) {
        if (lighting_shader_update_light_source(lighting_shader, i,
                                                Vector3Zero()))
            break;
    }
}

void update_shader_data(void) {
    lighting_shader_update(&lighting_scene.base_shader);
    lighting_shader_update(&lighting_scene.terrain_shader);
}

void light_source_update(LightSourceHandle light_handle, Vector3 offset) {
    lighting_shader_update_light_source(&lighting_scene.base_shader,
                                        light_handle, offset);
    lighting_shader_update_light_source(&lighting_scene.terrain_shader,
                                        light_handle, offset);
}
