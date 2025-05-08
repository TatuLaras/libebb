#include "lighting.h"

#include "handles.h"
#include "lighting_edit.h"
#include "model_vector.h"
#include "scene.h"
#include "transform.h"
#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STARTING_SIZE 4
#define GROWTH_FACTOR 2

static const char *shader_path = "resources/shaders/vertex_lighting.vert";

LightingScene lighting_scene = {0};

static LightingGroupHandle lighting_scene_add_group(LightingGroup group) {
    if (lighting_scene.groups_allocated >= lighting_scene.groups_size) {
        lighting_scene.groups_allocated *= 2;
        lighting_scene.groups =
            realloc(lighting_scene.groups,
                    lighting_scene.groups_allocated * sizeof(LightingGroup));
        assert(lighting_scene.groups);
    }

    lighting_scene.groups[lighting_scene.groups_size++] = group;
    return lighting_scene.groups_size - 1;
}

void update_shader_data(LightingGroupHandle handle) {
    LightingGroup *group = lighting_scene_get_group(handle);
    if (!group)
        return;

    float ambient_color[4] = {(float)group->ambient_color.r / (float)255,
                              (float)group->ambient_color.g / (float)255,
                              (float)group->ambient_color.b / (float)255,
                              (float)group->ambient_color.a / (float)255};
    SetShaderValue(group->shader, group->ambient_color_location, &ambient_color,
                   SHADER_UNIFORM_VEC4);

    SetShaderValue(group->shader, group->is_shading_disabled_location,
                   &group->is_shading_disabled, SHADER_UNIFORM_INT);

    for (size_t i = 0; i < LIGHTING_MAX_LIGHTS_PER_GROUP; i++) {
        if (light_source_update(handle, i))
            break;
    }
}

LightingGroup *lighting_scene_get_group(LightingGroupHandle handle) {
    if (handle >= lighting_scene.groups_size)
        return 0;

    return lighting_scene.groups + handle;
}

int light_source_update(LightingGroupHandle group_handle,
                        LightSourceHandle light_handle) {
    LightingGroup *group = lighting_scene_get_group(group_handle);

    assert(group);
    if (!group || light_handle >= group->lights_size)
        return 1;

    LightSource *light = group->lights + light_handle;
    if (light->type == LIGHT_NULL)
        return 1;

    int enabled = !light->is_disabled;
    SetShaderValue(group->shader, light->is_enabled_location, &enabled,
                   SHADER_UNIFORM_INT);

    float intensity = light->intensity + light->intensity_granular;
    SetShaderValue(group->shader, light->intensity_location, &intensity,
                   SHADER_UNIFORM_FLOAT);

    SetShaderValue(group->shader, light->intensity_cap_location,
                   &light->intensity_cap, SHADER_UNIFORM_FLOAT);
    SetShaderValue(group->shader, light->type_location, &light->type,
                   SHADER_UNIFORM_INT);

    // If a transform is being performed we want to preview the effects of the
    // lighting before committing, hence this.
    Vector3 light_pos = light->position;
    if (lighting_edit_state.is_light_selected &&
        lighting_edit_state.currently_selected_light == light_handle &&
        transform_operation.mode != TRANSFORM_NONE) {
        light_pos =
            Vector3Add(light_pos, lighting_edit_transform_get_delta_vector());
    }
    float position[3] = {light_pos.x, light_pos.y, light_pos.z};
    SetShaderValue(group->shader, light->position_location, position,
                   SHADER_UNIFORM_VEC3);

    float target[3] = {light->target.x, light->target.y, light->target.z};
    SetShaderValue(group->shader, light->target_location, target,
                   SHADER_UNIFORM_VEC3);

    float color[4] = {
        (float)light->color.r / (float)255, (float)light->color.g / (float)255,
        (float)light->color.b / (float)255, (float)light->color.a / (float)255};
    SetShaderValue(group->shader, light->color_location, color,
                   SHADER_UNIFORM_VEC4);

    return 0;
}

void lighting_scene_init(void) {
    lighting_scene =
        (LightingScene){.groups = malloc(STARTING_SIZE * sizeof(LightingGroup)),
                        .groups_allocated = STARTING_SIZE};
}

void lighting_scene_free(void) {
    if (lighting_scene.groups)
        free(lighting_scene.groups);
}

LightingGroupHandle lighting_group_create(Color ambient_color) {
    LightingGroup group = {.shader = LoadShader(shader_path, 0),
                           .ambient_color = ambient_color};

    group.ambient_color_location = GetShaderLocation(group.shader, "ambient");
    group.is_shading_disabled_location =
        GetShaderLocation(group.shader, "shadingDisabled");

    return lighting_scene_add_group(group);
}

void lighting_group_add_entity(LightingGroupHandle group_handle,
                               Entity *entity) {
    LightingGroup *group = lighting_scene_get_group(group_handle);
    if (!group)
        return;

    entity->lighting_group_handle = group_handle;
    ModelData *model_data = scene_entity_get_model(entity);
    assert(model_data->model.meshCount);
    model_data->model.materials[0].shader = group->shader;
}

int lighting_group_add_light(LightingGroupHandle group_handle,
                             LightSource light,
                             LightSourceHandle *out_light_source_handle) {
    LightingGroup *group = lighting_scene_get_group(group_handle);
    assert(group);
    if (!group || group->lights_size >= LIGHTING_MAX_LIGHTS_PER_GROUP)
        return 1;

    uint32_t i = group->lights_size;

    // Connect shader locations
    light.color_location =
        GetShaderLocation(group->shader, TextFormat("lights[%i].color", i));
    light.is_enabled_location =
        GetShaderLocation(group->shader, TextFormat("lights[%i].enabled", i));
    light.intensity_location =
        GetShaderLocation(group->shader, TextFormat("lights[%i].intensity", i));
    light.intensity_cap_location = GetShaderLocation(
        group->shader, TextFormat("lights[%i].intensity_cap", i));
    light.type_location =
        GetShaderLocation(group->shader, TextFormat("lights[%i].type", i));
    light.position_location =
        GetShaderLocation(group->shader, TextFormat("lights[%i].position", i));
    light.target_location =
        GetShaderLocation(group->shader, TextFormat("lights[%i].target", i));

    if (out_light_source_handle)
        *out_light_source_handle = group->lights_size;

    group->lights[group->lights_size++] = light;

    update_shader_data(group_handle);

    return 0;
}

LightSource *lighting_group_get_light(LightingGroupHandle group_handle,
                                      LightSourceHandle light_handle) {
    LightingGroup *group = lighting_scene_get_group(group_handle);
    if (!group || light_handle >= group->lights_size)
        return 0;
    return group->lights + light_handle;
}

void lighting_group_set_enabled(LightingGroupHandle group_handle,
                                uint32_t enabled) {
    LightingGroup *group = lighting_scene_get_group(group_handle);
    if (!group)
        return;
    group->is_shading_disabled = !enabled;

    update_shader_data(group_handle);
}

void lighting_scene_set_enabled(uint32_t enabled) {
    for (size_t i = 0; i < lighting_scene.groups_size; i++) {
        lighting_group_set_enabled(i, enabled);
    }
}
