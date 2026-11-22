/** @file
 * @brief Traverse an active complete structural subtree without recursion or code execution.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains hidden files
 * and unused descendants; [SRC-02](../../../../../spec/language/source-and-names.md#src-02)
 * makes unsupported links and repeated directories explicit failures.
 */
#include "internal.h"
#include <string.h>
/** Pending directory paths borrow stable arena-owned memberships. Each admitted
 * directory enters the queue once under [PKG-07](../../../../../spec/project/packages.md#pkg-07).
 */
typedef struct Directory {
    struct Directory *next;
    const char *path;
} Directory;
SourceStatus acquisition_unit_tree(AcquisitionUnitCapture *capture, const char *parent, const char *name) {
    AcquisitionUnitEntry *root = NULL;
    SourceStatus status = acquisition_unit_child(capture, parent, name, true, true, &root);
    if (status != SOURCE_OK || !root) { return status; }
    return acquisition_unit_walk(capture, root->file.path);
}
SourceStatus acquisition_unit_walk(AcquisitionUnitCapture *capture, const char *path) {
    Directory initial = {.path = path}, *pending = &initial, **tail = &initial.next;
    SourceStatus status;
    while (pending) {
        const char *directory = pending->path;
        pending = pending->next; if (!pending) { tail = &pending; }
        const char **names = NULL; size_t count = 0;
        capture->failed = directory;
        ModuleStatus listed = module_catalog_entries(&capture->entries, directory, &names, &count);
        if (listed != MODULE_OK) { return listed == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR; }
        for (size_t index = 0; index < count; index++) {
            const char *path = source_arena_join(capture->arena, directory,
                directory[strlen(directory) - 1] == '/' ? "" : "/", names[index]);
            if (!path) { return SOURCE_OUT_OF_MEMORY; }
            capture->failed = path;
            struct stat metadata;
            if (path_exact_child(directory, names[index], path, &metadata) != PATH_FOUND) { return SOURCE_IO_ERROR; }
            bool is_directory = S_ISDIR(metadata.st_mode);
            AcquisitionUnitEntry *entry = NULL;
            status = acquisition_unit_child(capture, directory, names[index], false, is_directory, &entry);
            if (status != SOURCE_OK) { return status; }
            if (is_directory) {
                Directory *next = source_arena_allocate(capture->arena, sizeof(*next));
                if (!next) { return SOURCE_OUT_OF_MEMORY; }
                *next = (Directory){.path = entry->file.path}; *tail = next; tail = &next->next;
            }
        }
    }
    return SOURCE_OK;
}
