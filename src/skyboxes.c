#include "skyboxes.h"

#include "scene.h"
#include "settings.h"
#include "string_vector.h"
#include <assert.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

StringVector skybox_list = {0};

// Returns 1 if file ends in .aseprite
static inline int is_aseprite(char *filename) {
    size_t length = strlen(filename);
    if (length < 4)
        return 0;

    int holds_true = 1;
    holds_true = holds_true && filename[length - 9] == '.';
    holds_true = holds_true && filename[length - 8] == 'a';
    holds_true = holds_true && filename[length - 7] == 's';
    holds_true = holds_true && filename[length - 6] == 'e';
    holds_true = holds_true && filename[length - 5] == 'p';
    holds_true = holds_true && filename[length - 4] == 'r';
    holds_true = holds_true && filename[length - 3] == 'i';
    holds_true = holds_true && filename[length - 2] == 't';
    holds_true = holds_true && filename[length - 1] == 'e';

    return holds_true;
}

// Inserts a list of filenames (without extension) of all the .aseprite files in
// `directory` into the StringVector `destination`.
static inline void get_aseprite_basenames(const char *directory,
                                          StringVector *destination) {
    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (!d) {
        perror("Could not open directory");
        return;
    }

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG && is_aseprite(dir->d_name))
            // The "- 4" strips the .aseprite extension
            stringvec_append(destination, dir->d_name, strlen(dir->d_name) - 9);
    }

    closedir(d);

    return;
}

void skyboxes_fetch_all(void) {
    if (!skybox_list.data)
        skybox_list = stringvec_init();

    if (*settings.project_directory == 0)
        return;

    stringvec_truncate(&skybox_list);

    char skybox_directory[MAX_PATH_LENGTH + 1] = {0};
    strcpy(skybox_directory, settings.project_directory);
    strcat(skybox_directory, "skyboxes/");

    get_aseprite_basenames(skybox_directory, &skybox_list);
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

size_t skyboxes_get_count(void) {
    return skybox_list.indices_used;
}
