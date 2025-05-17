#ifndef _FILESYSTEM
#define _FILESYSTEM

#include "string_vector.h"

// Inserts a list of filenames (without extension) of all files in `directory`
// that end with `suffix` into the StringVector `destination`.
void get_basenames_with_suffix(const char *directory, StringVector *destination,
                               const char *suffix);

#endif
