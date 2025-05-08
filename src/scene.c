#include "scene.h"

#include "assets.h"
#include "handles.h"
#include "lighting.h"
#include "lighting_edit.h"
#include "model_files.h"
#include "raygui.h"
#include "settings.h"
#include "skyboxes.h"
#include "ui.h"
#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENTITIES_STARTING_SIZE 4
#define ENTITIES_GROWTH_FACTOR 2

Scene scene = {0};
static Model skybox_model = {0};

// Loads a model of asset name `asset_name` from the asset directory (see
// settings.h).
static inline Model load_asset_model(AssetHandle handle) {
    char *asset_name = assets_get_name(handle);
    assert(asset_name);

    char filepath[MAX_PATH_LENGTH + 1] = {0};

    strcpy(filepath, settings.project_directory);
    strcat(filepath, "assets/");
    strcat(filepath, asset_name);
    strcat(filepath, ".glb");

    Model model = LoadModel(filepath);
    if (!model.meshCount)
        return (Model){0};
    try_load_corresponding_texture(filepath, &model);
    return model;
}

void scene_init(void) {
    scene = (Scene){
        .models = modelvec_init(),
        .entities = malloc(ENTITIES_STARTING_SIZE * sizeof(Entity)),
        .entities_allocated = ENTITIES_STARTING_SIZE,
    };
}

int scene_add(Entity entity, EntityHandle *out_entity_handle) {

    if (scene.entities_used >= scene.entities_allocated) {
        // Grow buffer
        scene.entities_allocated *= ENTITIES_GROWTH_FACTOR;
        scene.entities =
            realloc(scene.entities, scene.entities_allocated * sizeof(Entity));
        if (!scene.entities)
            return 1;
    }

    char *asset_name = assets_get_name(entity.asset_handle);
    assert(asset_name);

    // Check if model of entity already loaded, connect index
    size_t i = 0;
    Entity *scene_entity = 0;
    int match_found = 0;
    while ((scene_entity = scene_get_entity(i++))) {
        char *scene_entity_asset_name =
            assets_get_name(scene_entity->asset_handle);
        assert(scene_entity_asset_name);

        if (!strcmp(asset_name, scene_entity_asset_name)) {
            // Needs to not be a private instance of a model
            ModelData *model_data =
                modelvec_get(&scene.models, scene_entity->model_handle);
            assert(model_data);

            entity.model_handle = scene_entity->model_handle;
            match_found = 1;
            break;
        }
    }

    // If not, load model
    if (!match_found) {
        ModelData model_data = {.model = load_asset_model(entity.asset_handle)};
        if (!model_data.model.meshCount)
            return 1;

        entity.model_handle = modelvec_append(&scene.models, model_data);
    }

    scene.entities[scene.entities_used++] = entity;

    if (out_entity_handle)
        *out_entity_handle = scene.entities_used - 1;

    return 0;
}

void scene_remove(EntityHandle handle) {
    if (handle < scene.entities_used)
        scene.entities[handle].is_destroyed = 1;
}

Entity *scene_get_entity(EntityHandle handle) {
    if (handle >= scene.entities_used)
        return 0;
    return scene.entities + handle;
}

void scene_free(void) {
    size_t i = 0;
    ModelData *model_data = 0;
    while ((model_data = modelvec_get(&scene.models, i++)))
        UnloadModel(model_data->model);

    modelvec_free(&scene.models);
    if (scene.entities)
        free(scene.entities);
}

ModelData *scene_entity_get_model(Entity *entity) {
    return modelvec_get(&scene.models, entity->model_handle);
}

void scene_render_properties_menu(void) {

    uint16_t title_height = 20;
    uint16_t picker_height = 120;
    uint16_t spacing_height = 10;

    LightingGroup *lighting_group =
        lighting_scene_get_group(lighting_edit_state.current_group);

    Rectangle title_rect = ui_properties_menu_reserve_height(title_height);
    GuiLabel(title_rect, "Scene");

    // Color picker
    Color before = lighting_group->ambient_color;
    Rectangle color_picker_rect =
        ui_properties_menu_reserve_height(picker_height);
    color_picker_rect.width -= 32;
    color_picker_rect.x += 2;
    color_picker_rect.y += 4;
    GuiColorPicker(color_picker_rect, 0, &lighting_group->ambient_color);
    Color after = lighting_group->ambient_color;

    if (before.r != after.r || before.b != after.b || before.g != after.g ||
        before.a != after.a)
        update_shader_data(lighting_edit_state.current_group);

    ui_properties_menu_reserve_height(spacing_height);

    ui_properties_menu_reserve_section_end();
}

void scene_load_skybox(void) {
    if (!skybox_model.meshCount)
        skybox_model = LoadModel("resources/skybox.obj");

    char *skybox_name = skyboxes_get_name(scene.skybox_handle);
    assert(skybox_name);

    char path[MAX_PATH_LENGTH] = {0};
    strcpy(path, settings.project_directory);
    strcat(path, "skyboxes/");
    strcat(path, skybox_name);
    strcat(path, ".aseprite");
    load_aseprite_texture(path, &skybox_model);
}

void scene_set_skybox(SkyboxHandle handle) {
    scene.skybox_handle = handle;
    scene_load_skybox();
}

void scene_render_skybox(Camera3D camera) {
    BeginMode3D(camera);
    DrawModel(skybox_model, camera.position, 1.0, WHITE);
    EndMode3D();
}
