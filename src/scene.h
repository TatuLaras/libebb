#ifndef _SCENE
#define _SCENE

#include "handles.h"
#include "model_vector.h"
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_PATH_LENGTH 4096

// Base entity data
typedef struct {
    AssetHandle asset_handle;
    LightingGroupHandle lighting_group_handle;
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

extern Scene scene;

void scene_init(void);
void scene_free(void);

// Adds a new entity to the scene. Assumes a raylib context is already
// initialized. Returns 1 on error.
int scene_add(Entity entity, EntityHandle *out_entity_handle);
// Removes an entity from the scene by `handle`.
void scene_remove(EntityHandle handle);
// Gets entity of `scene` by `id`, returns 0 when no entity for that index
// exists.
Entity *scene_get_entity(EntityHandle handle);
// Gets the model of an entity.
ModelData *scene_entity_get_model(Entity *entity);
// Loads the currently set skybox as scene background.
void scene_load_skybox(void);
// Renders the current skybox as scene background.
void scene_render_skybox(Camera3D camera);
// Sets the current skybox
void scene_set_skybox(SkyboxHandle handle);

void scene_render_properties_menu(void);

#endif
