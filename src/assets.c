#include "assets.h"

#include "scene.h"
#include "settings.h"
#include "string_vector.h"
#include <assert.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

StringVector asset_list = {0};

// Returns 1 if file ends in .glb.
static inline int is_glb(char *filename) {
    size_t length = strlen(filename);
    if (length < 4)
        return 0;

    int holds_true = 1;
    holds_true = holds_true && filename[length - 4] == '.';
    holds_true = holds_true && filename[length - 3] == 'g';
    holds_true = holds_true && filename[length - 2] == 'l';
    holds_true = holds_true && filename[length - 1] == 'b';

    return holds_true;
}

// Appends a list of filenames (without extension) of all the .glb files in
// `directory` into the StringVector `destination`.
static inline void get_glb_basenames(const char *directory,
                                     StringVector *destination) {
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (!d)
        return;

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG && is_glb(dir->d_name))
            // The "- 4" strips the .glb extension
            stringvec_append(destination, dir->d_name, strlen(dir->d_name) - 4);
    }

    closedir(d);

    return;
}

void assets_fetch_all(void) {
    if (!asset_list.data)
        asset_list = stringvec_init();

    if (*settings.project_directory == 0)
        return;

    stringvec_truncate(&asset_list);

    char asset_directory[MAX_PATH_LENGTH + 1] = {0};
    strcpy(asset_directory, settings.project_directory);
    strcat(asset_directory, "assets/");

    get_glb_basenames(asset_directory, &asset_list);
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
