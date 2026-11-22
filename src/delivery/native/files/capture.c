/** @file
 * @brief Capture raw optional-file bytes without executing authored instructions.
 * [BUILD-01](../../../../spec/implementation/application-builds.md#build-01)
 * excludes CMake execution from generation; [BUILD-05](../../../../spec/implementation/application-builds.md#build-05)
 * retains an unavailable guidance path instead of silently treating it as absent.
 */
#include <string.h>
#include "snapshot.h"

SourceStatus delivery_file_capture(SourceArena *arena, SourceManager *sources,
    const char *parent, const char *name, DeliveryFileSnapshot *output) {
    if (arena == NULL || sources == NULL || parent == NULL || *parent == 0
        || name == NULL || *name == 0 || strchr(name, '/') != NULL
        || strcmp(name, ".") == 0 || strcmp(name, "..") == 0 || output == NULL) {
        return SOURCE_INVALID_ARGUMENT;
    }
    const char *path = source_arena_join(arena, parent,
        parent[strlen(parent) - 1] == '/' ? "" : "/", name);
    if (path == NULL) { return SOURCE_OUT_OF_MEMORY; }
    struct stat metadata;
    PathProbe probe = path_exact_child(parent, name, path, &metadata);
    DeliveryFileSnapshot prepared = {.path = path, .issue = probe};
    if (probe == PATH_MISSING) {
        prepared.state = DELIVERY_FILE_ABSENT;
    } else if (probe != PATH_FOUND || !S_ISREG(metadata.st_mode)) {
        prepared.state = DELIVERY_FILE_UNAVAILABLE;
        if (probe == PATH_FOUND) { prepared.issue = PATH_UNSUPPORTED; }
    } else {
        SourceStatus status = source_manager_load(sources, path, &prepared.content);
        if (status == SOURCE_IO_ERROR) {
            prepared.state = DELIVERY_FILE_UNAVAILABLE;
            prepared.issue = PATH_IO_ERROR;
        } else if (status != SOURCE_OK) {
            return status;
        } else {
            prepared.state = DELIVERY_FILE_PRESENT;
        }
    }
    *output = prepared;
    return SOURCE_OK;
}
