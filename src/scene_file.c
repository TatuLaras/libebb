#include "scene_file.h"

#include "assets.h"
#include "common.h"
#include "general_buffer.h"
#include "handles.h"
#include "lighting.h"
#include "scene.h"
#include "skyboxes.h"
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
                                 uint16_t *out_light_group_table_entries,
                                 size_t *out_light_source_table_entries) {
    GeneralBuffer light_sources = genbuf_init();
    LightingGroup *group = 0;
    LightSource *light = 0;
    LightingGroupHandle group_handle = 0;
    LightSourceHandle light_handle = 0;
    uint16_t lighting_group_table_entries = 0;
    size_t light_source_table_entries = 0;
    while ((group = lighting_scene_get_group(group_handle))) {
        if (group->is_deleted)
            continue;

        SceneFileLightingGroup scene_file_group = {.ambient_color =
                                                       group->ambient_color};
        genbuf_append(buf, &scene_file_group, sizeof(SceneFileLightingGroup));

        // Light sources
        light_handle = 0;
        while ((light = lighting_group_get_light(group_handle, light_handle))) {
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
            genbuf_append(&light_sources, &scene_file_light,
                          sizeof(SceneFileLightSource));
            light_source_table_entries++;
            light_handle++;
        }

        lighting_group_table_entries++;
        group_handle++;
    }

    genbuf_append(buf, light_sources.data, light_sources.data_size);
    genbuf_free(&light_sources);

    if (out_light_group_table_entries)
        *out_light_group_table_entries = lighting_group_table_entries;
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
    char *skybox_name = skyboxes_get_name(scene.skybox_handle);
    SceneFileSkybox skybox = {0};
    strncpy(skybox.name, skybox_name, ARRAY_LENGTH(skybox.name) - 1);
    genbuf_append(buf, &skybox, (sizeof skybox));

    if (out_skybox_entry_count)
        *out_skybox_entry_count = 1;
}

void scene_file_store(FILE *fp) {
    printf("INFO: saving scene file.\n");

    GeneralBuffer content = genbuf_init();

    size_t asset_table_entries = 0;
    serialize_asset_data_into_buf(&content, &asset_table_entries);

    uint16_t lighting_group_table_entries = 0;
    size_t light_source_table_entries = 0;
    serialize_lighting_data_into_buf(&content, &lighting_group_table_entries,
                                     &light_source_table_entries);

    size_t entity_table_entries = 0;
    serialize_entity_data_into_buf(&content, &entity_table_entries);

    size_t skybox_entry_count = 0;
    serialize_skybox_data_into_buf(&content, &skybox_entry_count);

    SceneFileHeader header = {
        .magic = SCENE_FILE_MAGIC,

        .asset_count = asset_table_entries,
        .lighting_group_count = lighting_group_table_entries,
        .light_source_count = light_source_table_entries,
        .entity_count = entity_table_entries,
        .skybox_count = skybox_entry_count,

        .header_size = sizeof(SceneFileHeader),
        .asset_size = sizeof(SceneFileAsset),
        .lighting_group_size = sizeof(SceneFileLightingGroup),
        .light_source_size = sizeof(SceneFileLightSource),
        .entity_size = sizeof(SceneFileEntity),
        .skybox_size = sizeof(SceneFileSkybox),
    };

    fwrite(&header, (sizeof header), 1, fp);
    fwrite(content.data, content.data_size, 1, fp);
    genbuf_free(&content);
}

int scene_file_load(FILE *fp) {
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
    if (header_ptr->header_size > sizeof(SceneFileHeader)) {
        free(file_buffer);
        return 1;
    }

    //  NOTE: In case a struct has more stuff added onto it later on,
    //  this way we can still read old files and just have the new fields be
    //  zero (we're assuming zero is initialization for new struct fields).
    SceneFileHeader header = {0};
    memcpy(&header, file_buffer, header_ptr->header_size);
    offset += header.header_size;

    size_t assumed_file_size =
        header.header_size + header.asset_size * header.asset_count +
        header.lighting_group_size * header.lighting_group_count +
        header.light_source_size * header.light_source_count +
        header.entity_size * header.entity_count +
        header.skybox_size * header.skybox_count;

    if (file_size != assumed_file_size) {
        free(file_buffer);
        return 1;
    }

    char *asset_table = (char *)offset;

    offset += header.asset_size * header.asset_count;

    for (size_t i = 0; i < header.lighting_group_count; i++) {
        SceneFileLightingGroup group = {0};
        memcpy(&group, offset + i * header.lighting_group_size,
               header.lighting_group_size);

        LightingGroup *default_group = lighting_scene_get_group(0);
        default_group->ambient_color = group.ambient_color;
    }

    offset += header.lighting_group_size * header.lighting_group_count;

    for (size_t i = 0; i < header.light_source_count; i++) {

        SceneFileLightSource light = {0};
        memcpy(&light, offset + i * header.light_source_size,
               header.light_source_size);

        lighting_group_add_light(
            0,
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
        scene_add(
            (Entity){
                .asset_handle = asset_handle,
                .transform = entity.transform,
                .ignore_raycast = entity.ignore_raycast,
            },
            &entity_handle);

        lighting_group_add_entity(0, scene_get_entity(entity_handle));

        Entity *added_entity = scene_get_entity(entity_handle);
        assert(added_entity);
    }

    offset += header.entity_size * header.entity_count;

    if (header.skybox_count > 0) {
        SceneFileSkybox skybox = {0};
        memcpy(&skybox, offset, header.skybox_size);

        SkyboxHandle skybox_handle = 0;
        if (!skyboxes_get_handle(skybox.name, &skybox_handle)) {
            scene_set_skybox(skybox_handle);
        } else
            fprintf(stderr, "WARNING: Skybox %s no longer exists.\n",
                    skybox.name);
    }

    free(file_buffer);
    return 0;
}
