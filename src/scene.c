#include "scene.h"

#include "assets.h"
#include "common.h"
#include "firewatch.h"
#include "handles.h"
#include "lighting.h"
#include "skyboxes.h"
#include "texture_load.h"
#include <assert.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENTITIES_STARTING_SIZE 4
#define ENTITIES_GROWTH_FACTOR 2

static Scene scene = {0};
static Model skybox_model = {0};

static inline void load_model(const char *filepath, ModelHandle handle) {
    // Preserve textures
    Texture texture = {0};
    Texture unlit_texture = {0};
    if (scene.models.data[handle].materialCount) {
        texture = scene.models.data[handle]
                      .materials[0]
                      .maps[MATERIAL_MAP_DIFFUSE]
                      .texture;
        unlit_texture = scene.models.data[handle]
                            .materials[0]
                            .maps[MATERIAL_MAP_DIFFUSE + 1]
                            .texture;
    }

    if (scene.models.data[handle].meshes)
        UnloadModel(scene.models.data[handle]);

    scene.models.data[handle] = LoadModel(filepath);
    assert(scene.models.data[handle].meshes);
    scene.models.data[handle].materials[0].shader =
        lighting_scene_get_base_shader();

    // Load old textures
    scene.models.data[handle].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
        texture;
    scene.models.data[handle]
        .materials[0]
        .maps[MATERIAL_MAP_DIFFUSE + 1]
        .texture = unlit_texture;
}

static inline void load_texture(const char *filepath, ModelHandle handle) {
    Model *model = modelvec_get(&scene.models, handle);
    assert(model);
    assert(model->materialCount);
    assert(model->meshCount);
    texture_load_model_texture(filepath, model);
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
            Model *model_data =
                modelvec_get(&scene.models, scene_entity->model_handle);
            assert(model_data);

            entity.model_handle = scene_entity->model_handle;
            match_found = 1;
            break;
        }
    }

    // If not, load model
    if (!match_found) {
        entity.model_handle = modelvec_append(&scene.models, (Model){0});

        char *asset_filename = assets_get_name(entity.asset_handle);
        assert(asset_filename);

        char texture_filepath[MAX_PATH_LENGTH] = {0};
        char model_filepath[MAX_PATH_LENGTH] = {0};

        strcpy(texture_filepath, asset_directory);
        strcat(texture_filepath, asset_filename);
        strcpy(model_filepath, asset_directory);
        strcat(model_filepath, asset_filename);

        strcat(model_filepath, ".glb");
        strcat(texture_filepath, ".aseprite");

#ifndef NO_HOT_RELOAD
        firewatch_new_file_ex(model_filepath, 0, entity.model_handle,
                              LOAD_KIND_MODEL);
        firewatch_new_file_ex(texture_filepath, 0, entity.model_handle,
                              LOAD_KIND_TEXTURE);
#endif

        load_model(model_filepath, entity.model_handle);
        load_texture(texture_filepath, entity.model_handle);

        scene_check_for_model_file_updates();
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

void scene_check_for_model_file_updates(void) {
    static LoadRequest request = {0};
    while (firewatch_request_stack_pop(&request)) {
        switch (request.kind) {
        case LOAD_KIND_MODEL:
            load_model(request.filepath, request.cookie);
            break;
        case LOAD_KIND_TEXTURE:
            load_texture(request.filepath, request.cookie);
            break;
        default:
            break;
        }
    }
}

void scene_free(void) {
    size_t i = 0;
    Model *model = 0;
    while ((model = modelvec_get(&scene.models, i++)))
        UnloadModel(*model);

    modelvec_free(&scene.models);
    if (scene.entities)
        free(scene.entities);
}

Model *scene_entity_get_model(Entity *entity) {
    return modelvec_get(&scene.models, entity->model_handle);
}

void scene_skybox_init(const char *skybox_model_path) {
    skybox_model = LoadModel(skybox_model_path);
}

void scene_load_selected_skybox(const char *skybox_directory) {
    assert(skybox_model.meshCount);

    char *skybox_name = skyboxes_get_name(scene.skybox_handle);
    assert(skybox_name);

    char path[MAX_PATH_LENGTH] = {0};
    strcpy(path, skybox_directory);
    strcat(path, skybox_name);
    strcat(path, ".aseprite");

    Texture skybox_texture = texture_load_aseprite_texture(path);
    if (skybox_texture.id)
        skybox_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            skybox_texture;
}

void scene_set_skybox(SkyboxHandle handle, const char *skybox_directory) {
    scene.skybox_handle = handle;
    scene_load_selected_skybox(skybox_directory);
}

SkyboxHandle scene_get_skybox(void) {
    return scene.skybox_handle;
}

void scene_render_skybox(Camera3D camera) {
    BeginMode3D(camera);
    DrawModel(skybox_model, camera.position, 1.0, WHITE);
    EndMode3D();
}
