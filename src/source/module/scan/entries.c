#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"

typedef struct EntryName {
    struct EntryName *next;
    const char *name;
} EntryName;

static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}

static ModuleStatus remember_names(ModuleWalk *walk, DIR *stream,
    EntryName **first, size_t *count) {
    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(stream)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        EntryName *name = source_arena_allocate(&walk->catalog->arena, sizeof(*name));
        const char *copy = source_arena_copy(&walk->catalog->arena, entry->d_name);
        if (name == NULL || copy == NULL || *count == SIZE_MAX / sizeof(char *)) {
            return MODULE_OUT_OF_MEMORY;
        }
        *name = (EntryName){*first, copy};
        *first = name;
        (*count)++;
        errno = 0;
    }
    return errno == 0 ? MODULE_OK : MODULE_IO_ERROR;
}

ModuleStatus module_read_entries(ModuleWalk *walk, const ModuleDirectory *directory,
    const char ***names, size_t *count) {
    DIR *stream = opendir(directory->physical);
    if (stream == NULL) {
        return module_fail(walk->catalog, MODULE_IO_ERROR, directory->physical);
    }
    EntryName *first = NULL;
    *count = 0;
    ModuleStatus status = remember_names(walk, stream, &first, count);
    int closed = closedir(stream);
    if (status != MODULE_OK || closed != 0) {
        return module_fail(walk->catalog, status != MODULE_OK ? status : MODULE_IO_ERROR,
            directory->physical);
    }
    const char **sorted = source_arena_allocate(&walk->catalog->arena, *count * sizeof(*sorted));
    if (sorted == NULL) {
        return module_fail(walk->catalog, MODULE_OUT_OF_MEMORY, directory->physical);
    }
    for (size_t index = 0; first != NULL; first = first->next) {
        sorted[index++] = first->name;
    }
    qsort(sorted, *count, sizeof(*sorted), compare_names);
    *names = sorted;
    return MODULE_OK;
}
