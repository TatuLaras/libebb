#include "skyboxes.h"

#include "filesystem.h"
#include "string_vector.h"
#include <assert.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>

StringVector skybox_list = {0};

void skyboxes_fetch_all(const char *skybox_directory) {
    if (!skybox_list.data)
        skybox_list = stringvec_init();

    stringvec_truncate(&skybox_list);

    get_basenames_with_suffix(skybox_directory, &skybox_list, ".aseprite");
}

char *skyboxes_get_name(SkyboxHandle handle) {
    return stringvec_get(&skybox_list, handle);
}

int skyboxes_get_handle(const char *name, SkyboxHandle *out_handle) {
    int64_t index = stringvec_index_of(&skybox_list, name);
    if (index < 0)
        return 1;
    if (out_handle)
        *out_handle = index;
    return 0;
}
