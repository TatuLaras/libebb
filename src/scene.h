#ifndef _SCENE
#define _SCENE

#include "handles.h"
#include "model_vector.h"
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdint.h>

// Base entity data
typedef struct {
    AssetHandle asset_handle;
    ModelHandle model_handle;
    Matrix transform;
    int is_destroyed;
    int ignore_raycast;
} Entity;

typedef struct {
    Entity *entities;
    size_t entities_used;
    size_t entities_allocated;
    ModelVector models;
    SkyboxHandle skybox_handle;
} Scene;

void scene_init(void);
void scene_free(void);

// Adds a new entity to the scene. Assumes a raylib context is already
// initialized. Returns 1 on error.
int scene_add(Entity entity, EntityHandle *out_entity_handle,
              const char *asset_directory);
// Removes an entity from the scene by `handle`.
void scene_remove(EntityHandle handle);
// Gets entity of `scene` by `id`, returns 0 when no entity for that index
// exists.
Entity *scene_get_entity(EntityHandle handle);
// Gets the model of an entity.
Model *scene_entity_get_model(Entity *entity);

// Initializes the skybox, call this before any other skybox functions.
void scene_skybox_init(const char *skybox_model_path);
// Loads the currently set skybox as scene background.
void scene_load_selected_skybox(const char *skybox_directory);
// Renders the current skybox as scene background.
void scene_render_skybox(Camera3D camera);

void scene_set_skybox(SkyboxHandle handle, const char *skybox_directory);
SkyboxHandle scene_get_skybox(void);

void scene_render_properties_menu(void);

// Call this every frame to enable hot-reload functionality.
void scene_check_for_model_file_updates(void);

#endif
