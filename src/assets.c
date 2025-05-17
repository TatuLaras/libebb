#include "assets.h"

#include "filesystem.h"
#include "string_vector.h"
#include <assert.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

StringVector asset_list = {0};

void assets_fetch_all(const char *asset_directory) {
    if (!asset_list.data)
        asset_list = stringvec_init();

    stringvec_truncate(&asset_list);

    get_basenames_with_suffix(asset_directory, &asset_list, ".glb");
}

char *assets_get_name(AssetHandle handle) {
    return stringvec_get(&asset_list, handle);
}

int assets_get_handle(const char *name, AssetHandle *out_handle) {
    int64_t index = stringvec_index_of(&asset_list, name);
    if (index < 0)
        return 1;
    if (out_handle)
        *out_handle = index;
    return 0;
}

size_t assets_get_count(void) {
    return asset_list.indices_used;
}
