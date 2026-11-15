/**
 * @file
 * @brief Collect and sort actual directory names with checked allocation and stream cleanup.
 *
 * [COMP-05](../../../../spec/implementation/compiler.md#comp-05)
 * Sorting exact bytes makes discovery independent of filesystem enumeration order.
 */
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../internal.h"

/** Arena-owned copy of an unstable readdir name, later flattened into a sorted array. */
typedef struct EntryName {
    struct EntryName *next;
    const char *name;
} EntryName;

/** Order borrowed byte strings with strcmp for reproducible discovery; no Unicode normalization or locale collation.
 */
static int compare_names(const void *left, const void *right) {
    return strcmp(*(const char *const *)left, *(const char *const *)right);
}

/** Copy each non-dot readdir entry before the next readdir call. Checked count/allocation/read failure leaves scratch in catalog arena.
 */
static ModuleStatus remember_names(ModuleCatalog *catalog, DIR *stream,
    EntryName **first, size_t *count) {
    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(stream)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        EntryName *name = source_arena_allocate(&catalog->arena, sizeof(*name));
        const char *copy = source_arena_copy(&catalog->arena, entry->d_name);
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

ModuleStatus module_catalog_entries(ModuleCatalog *catalog, const char *physical,
    const char ***names, size_t *count) {
    DIR *stream = opendir(physical);
    if (stream == NULL) {
        return module_fail(catalog, MODULE_IO_ERROR, physical);
    }
    EntryName *first = NULL;
    *count = 0;
    ModuleStatus status = remember_names(catalog, stream, &first, count);
    int closed = closedir(stream);
    if (status != MODULE_OK || closed != 0) {
        return module_fail(catalog, status != MODULE_OK ? status : MODULE_IO_ERROR,
            physical);
    }
    const char **sorted = source_arena_allocate(&catalog->arena, *count * sizeof(*sorted));
    if (sorted == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    for (size_t index = 0; first != NULL; first = first->next) {
        sorted[index++] = first->name;
    }
    qsort(sorted, *count, sizeof(*sorted), compare_names);
    *names = sorted;
    return MODULE_OK;
}
