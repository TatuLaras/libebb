#include "scene.h"

#include "assets.h"
#include "common.h"
#include "handles.h"
#include "model_files.h"
#include "skyboxes.h"
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
static inline Model load_asset_model(AssetHandle handle,
                                     const char *asset_directory) {
    char *asset_name = assets_get_name(handle);
    assert(asset_name);

    char filepath[MAX_PATH_LENGTH + 1] = {0};

    strcpy(filepath, asset_directory);
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

int scene_add(Entity entity, EntityHandle *out_entity_handle,
              const char *asset_directory) {

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
        ModelData model_data = {
            .model = load_asset_model(entity.asset_handle, asset_directory)};
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

void scene_load_skybox(const char *skybox_directory) {
    if (!skybox_model.meshCount) {
        char skybox_model_path[MAX_PATH_LENGTH + 1] = __FILE__;
        strip_filename(skybox_model_path, MAX_PATH_LENGTH);
        strcat(skybox_model_path, "../resources/skybox.obj");
        skybox_model = LoadModel(skybox_model_path);
    }

    char *skybox_name = skyboxes_get_name(scene.skybox_handle);
    assert(skybox_name);

    char path[MAX_PATH_LENGTH] = {0};
    strcpy(path, skybox_directory);
    strcat(path, skybox_name);
    strcat(path, ".aseprite");

    Texture skybox_texture = load_aseprite_texture(path);
    if (skybox_texture.id)
        skybox_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            skybox_texture;
}

void scene_set_skybox(SkyboxHandle handle, const char *skybox_directory) {
    scene.skybox_handle = handle;
    scene_load_skybox(skybox_directory);
}

void scene_render_skybox(Camera3D camera) {
    BeginMode3D(camera);
    DrawModel(skybox_model, camera.position, 1.0, WHITE);
    EndMode3D();
}
