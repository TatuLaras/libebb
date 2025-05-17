#ifndef _SCENE_FILE
#define _SCENE_FILE

/*
Functions for storing a scene as a file.

Scene file structure:
    - A header, as described by struct `SceneFileHeader`, which will specify the
    sizes (amount of entries) of the following sections.
    - A table of asset names
    - A table of light groups
    - A table of light sources
    - A table of entities
 */

//  NOTE: IMPORTANT! Never shrink the SceneFileWhatever structs, only grow.
//  If a member has to be removed replace it with empty space until you have
//  something else to put there.

#include "common.h"
#include "lighting.h"
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>

#define SCENE_FILE_MAGIC 0x1273
#define SCENE_FILE_VERSION 0

#include <stdint.h>

typedef struct {
    uint16_t reserved : 16;
} SceneFileFlags;

typedef struct {
    uint16_t magic;
    SceneFileFlags flags;
    uint16_t header_size;

    uint32_t asset_count;
    uint32_t light_source_count;
    uint64_t entity_count;

    uint16_t lighting_scene_size;
    uint16_t asset_size;
    uint16_t light_source_size;
    uint16_t entity_size;

    uint32_t skybox_count;
    uint16_t skybox_size;
} SceneFileHeader;

typedef struct {
    char name[NAME_MAX_LENGTH];
} SceneFileAsset;

typedef struct {
    Color ambient_color;
} SceneFileLightingScene;

typedef struct {
    Color color;
    uint8_t is_disabled;
    LightType type : 8;
    uint16_t light_group_index;
    float intensity;
    float intensity_granular;
    float intensity_cap;
    Vector3 position;
} SceneFileLightSource;

typedef struct {
    Matrix transform;
    uint32_t asset_index;
    uint32_t light_group_index;
    uint8_t ignore_raycast;
    uint8_t : 8;
} SceneFileEntity;

typedef struct {
    char name[NAME_MAX_LENGTH];
} SceneFileSkybox;

// Stores the current scene into a file.
void scene_file_store(FILE *fp);
// Loads scene information from a file and applies it to the current scene. An
// empty scene along with lighting groups needs to be initalized before calling
// this function.
// Currently assumes there is only one light group and it's handle is 0.
int scene_file_load(FILE *fp, const char *skybox_directory,
                    const char *asset_directory);

#endif
