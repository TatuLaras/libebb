#include "scene_file.h"

#include "assets.h"
#include "common.h"
#include "general_buffer.h"
#include "handles.h"
#include "lighting.h"
#include "scene.h"
#include "skyboxes.h"
#include "terrain.h"
#include "terrain_textures.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void
serialize_asset_data_into_buf(GeneralBuffer *buf,
                              size_t *out_asset_table_entries) {
    size_t asset_table_entries = assets_get_count();
    char *asset_table =
        genbuf_allocate(buf, sizeof(SceneFileAsset) * asset_table_entries);

    AssetHandle asset_handle = 0;
    char *asset_name = 0;
    while ((asset_name = assets_get_name(asset_handle)) &&
           asset_handle < asset_table_entries) {
        strncpy(asset_table + asset_handle * sizeof(SceneFileAsset), asset_name,
                NAME_MAX_LENGTH);
        asset_handle++;
    }

    if (out_asset_table_entries)
        *out_asset_table_entries = asset_table_entries;
}

static inline void
serialize_lighting_data_into_buf(GeneralBuffer *buf,
                                 size_t *out_light_source_table_entries) {
    // Lighting scene
    SceneFileLightingScene file_lighting_scene = {
        .ambient_color = lighting_scene_get_ambient_color()};
    genbuf_append(buf, &file_lighting_scene, sizeof(SceneFileLightingScene));

    // Light sources
    LightSource *light = 0;
    LightSourceHandle light_handle = 0;
    uint16_t lighting_group_table_entries = 0;
    size_t light_source_table_entries = 0;

    while ((light = lighting_scene_get_light(light_handle++))) {
        SceneFileLightSource scene_file_light = {
            .light_group_index = lighting_group_table_entries,
            .color = light->color,
            .is_disabled = light->is_disabled,
            .type = light->type,
            .intensity = light->intensity,
            .intensity_granular = light->intensity_granular,
            .intensity_cap = light->intensity_cap,
            .position = light->position,
        };
        genbuf_append(buf, &scene_file_light, sizeof(SceneFileLightSource));
        light_source_table_entries++;
    }

    if (out_light_source_table_entries)
        *out_light_source_table_entries = light_source_table_entries;
}

static inline void
serialize_entity_data_into_buf(GeneralBuffer *buf,
                               size_t *out_entity_table_entries) {
    EntityHandle entity_handle = 0;
    Entity *entity = 0;
    size_t entity_table_entries = 0;
    while ((entity = scene_get_entity(entity_handle++))) {
        if (entity->is_destroyed)
            continue;
        SceneFileEntity scene_file_entity = {
            .transform = entity->transform,
            .ignore_raycast = entity->ignore_raycast,
            .asset_index = entity->asset_handle,

        };
        genbuf_append(buf, &scene_file_entity, sizeof(SceneFileEntity));
        entity_table_entries++;
    }

    if (out_entity_table_entries)
        *out_entity_table_entries = entity_table_entries;
}

//  NOTE: Currently we are only storing one skybox, good to have futureproofing
//  though.
static inline void
serialize_skybox_data_into_buf(GeneralBuffer *buf,
                               size_t *out_skybox_entry_count) {
    char *skybox_name = skyboxes_get_name(scene_get_skybox());
    SceneFileSkybox skybox = {0};
    strncpy(skybox.name, skybox_name, ARRAY_LENGTH(skybox.name) - 1);
    genbuf_append(buf, &skybox, (sizeof skybox));

    if (out_skybox_entry_count)
        *out_skybox_entry_count = 1;
}

static inline void serialize_terrain_data_into_buf(GeneralBuffer *buf,
                                                   size_t *out_heights_size,
                                                   size_t *out_indices_size) {
    SceneFileTerrainInfo terrain_info = {
        .top_left_world_pos = terrain.top_left_world_pos,
        .width = terrain.width,
    };

    // Texture names
    for (size_t i = 0; i < TERRAIN_MAX_TEXTURES; i++) {
        TerrainTextureHandle handle = terrain_textures_get_slot_handle(i);
        char *texture_name = terrain_textures_get_name(handle);
        if (!texture_name)
            continue;
        strncpy((char *)(terrain_info.texture_names + i), texture_name,
                NAME_MAX_LENGTH - 1);
    }

    genbuf_append(buf, &terrain_info, (sizeof terrain_info));
    genbuf_append(buf, terrain.heights, terrain.size * sizeof(float));
    genbuf_append(buf, terrain.texture_indices, terrain.size);

    if (out_heights_size)
        *out_heights_size = terrain.size * sizeof(float);
    if (out_indices_size)
        *out_indices_size = terrain.size;
}

void scene_file_store(FILE *fp) {
    printf("INFO: saving scene file.\n");

    GeneralBuffer content = genbuf_init();

    size_t asset_table_entries = 0;
    serialize_asset_data_into_buf(&content, &asset_table_entries);

    size_t light_source_table_entries = 0;
    serialize_lighting_data_into_buf(&content, &light_source_table_entries);

    size_t entity_table_entries = 0;
    serialize_entity_data_into_buf(&content, &entity_table_entries);

    size_t skybox_entry_count = 0;
    serialize_skybox_data_into_buf(&content, &skybox_entry_count);

    size_t heights_size = 0;
    size_t indices_size = 0;
    serialize_terrain_data_into_buf(&content, &heights_size, &indices_size);

    SceneFileHeader header = {
        .magic = SCENE_FILE_MAGIC,

        .asset_count = asset_table_entries,
        .light_source_count = light_source_table_entries,
        .entity_count = entity_table_entries,
        .skybox_count = skybox_entry_count,

        .header_size = sizeof(SceneFileHeader),
        .asset_size = sizeof(SceneFileAsset),
        .lighting_scene_size = sizeof(SceneFileLightingScene),
        .light_source_size = sizeof(SceneFileLightSource),
        .entity_size = sizeof(SceneFileEntity),
        .skybox_size = sizeof(SceneFileSkybox),
        .terrain_info_size = sizeof(SceneFileTerrainInfo),
        .terrain_heights_size = heights_size,
        .terrain_texture_indices_size = indices_size,
    };

    fwrite(&header, (sizeof header), 1, fp);
    fwrite(content.data, content.data_size, 1, fp);
    genbuf_free(&content);
}

//  TODO: chop up
int scene_file_load(FILE *fp, const char *skybox_directory,
                    const char *asset_directory) {
    printf("INFO: loading scene file.\n");

    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size < sizeof(SceneFileHeader))
        return 1;

    uint8_t *file_buffer = malloc(file_size);
    assert(file_buffer);

    uint8_t *offset = file_buffer;

    if (!fread(file_buffer, file_size, 1, fp)) {
        free(file_buffer);
        return 1;
    }

    SceneFileHeader *header_ptr = (SceneFileHeader *)file_buffer;
    if (header_ptr->magic != SCENE_FILE_MAGIC) {
        free(file_buffer);
        return 1;
    };

    //  NOTE: In case a struct has more stuff added onto it later on,
    //  this way we can still read old files and just have the new fields be
    //  zero (we're assuming zero is initialization for new struct fields).
    SceneFileHeader header = {0};
    memcpy(&header, file_buffer, header_ptr->header_size);
    offset += header.header_size;

    size_t assumed_file_size =
        header.header_size + header.asset_size * header.asset_count +
        header.lighting_scene_size +
        header.light_source_size * header.light_source_count +
        header.entity_size * header.entity_count + header.terrain_info_size +
        header.terrain_heights_size + header.terrain_texture_indices_size +
        header.skybox_size * header.skybox_count;

    if (file_size != assumed_file_size) {
        free(file_buffer);
        return 1;
    }

    char *asset_table = (char *)offset;

    offset += header.asset_size * header.asset_count;

    SceneFileLightingScene file_lighting_scene = {0};
    memcpy(&file_lighting_scene, offset, header.lighting_scene_size);
    lighting_scene_set_ambient_color(file_lighting_scene.ambient_color);

    offset += header.lighting_scene_size;

    for (size_t i = 0; i < header.light_source_count; i++) {

        SceneFileLightSource light = {0};
        memcpy(&light, offset + i * header.light_source_size,
               header.light_source_size);

        lighting_scene_add_light(
            (LightSource){
                .is_disabled = light.is_disabled,
                .intensity = light.intensity,
                .intensity_granular = light.intensity_granular,
                .intensity_cap = light.intensity_cap,
                .type = LIGHT_POINT,
                .position = light.position,
                .color = light.color,
            },
            0);
    }

    offset += header.light_source_size * header.light_source_count;

    for (size_t i = 0; i < header.entity_count; i++) {

        SceneFileEntity entity = {0};
        memcpy(&entity, offset + i * header.entity_size, header.entity_size);

        char *asset_name = asset_table + entity.asset_index * header.asset_size;

        AssetHandle asset_handle = 0;
        if (assets_get_handle(asset_name, &asset_handle)) {
            fprintf(stderr, "WARNING: Asset %s no longer exists.\n",
                    asset_name);
            continue;
        }

        EntityHandle entity_handle = 0;
        if (scene_add(
                (Entity){
                    .asset_handle = asset_handle,
                    .transform = entity.transform,
                    .ignore_raycast = entity.ignore_raycast,
                },
                &entity_handle, asset_directory))
            return 1;

        Entity *added_entity = scene_get_entity(entity_handle);
        assert(added_entity);
        lighting_scene_add_entity(added_entity);
    }

    offset += header.entity_size * header.entity_count;

    if (header.skybox_count > 0) {
        SceneFileSkybox skybox = {0};
        memcpy(&skybox, offset, header.skybox_size);

        SkyboxHandle skybox_handle = 0;
        if (!skyboxes_get_handle(skybox.name, &skybox_handle)) {
            scene_set_skybox(skybox_handle, skybox_directory);
        } else
            fprintf(stderr, "WARNING: Skybox %s no longer exists.\n",
                    skybox.name);
    }

    offset += header.skybox_size * header.skybox_count;

    SceneFileTerrainInfo terrain_info = {0};
    memcpy(&terrain_info, offset, header.terrain_info_size);
    offset += header.terrain_info_size;

    if (terrain_info.width * terrain_info.width !=
            header.terrain_heights_size / sizeof(float) ||
        terrain_info.width * terrain_info.width !=
            header.terrain_texture_indices_size)
        return 1;

    if (terrain_info.width == 0)
        return 0;

    terrain_resize(terrain_info.width - 1);

    memcpy(terrain.heights, offset, header.terrain_heights_size);
    offset += header.terrain_heights_size;
    memcpy(terrain.texture_indices, offset,
           header.terrain_texture_indices_size);
    offset += header.terrain_texture_indices_size;

    // Terrain textures
    for (size_t i = 0; i < TERRAIN_MAX_TEXTURES; i++) {
        TerrainTextureHandle texture_handle = 0;
        if (terrain_textures_get_handle(terrain_info.texture_names[i],
                                        &texture_handle)) {
            printf("WARNING: terrain texture %s no longer exists\n",
                   terrain_info.texture_names[i]);
            continue;
        }
        terrain_textures_load_into_slot(texture_handle, i, 0);
    }

    free(file_buffer);
    return 0;
}
