/**
 * @file
 * @brief Enumerate one optional storage namespace without entering any version contents.
 *
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) permits either storage
 * tree to be absent. [PKG-02](../../../../spec/project/packages.md#pkg-02) defers
 * all module loading until the complete package-version choice is known.
 */
#include "internal.h"
#include <string.h>

/** Find the already owned root spelling rather than retaining the caller's string.
 * Root verification registered this exact path before either discovery pass.
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02) requires retained
 * failure context to remain stable after borrowed build inputs expire, including
 * an allocation failure that cannot make another diagnostic-path copy.
 */
static const char *verified_root(const PackageScan *scan) {
    for (const ModuleDirectoryIdentity *directory = scan->catalog->directories;
        directory != NULL; directory = directory->next) {
        if (strcmp(directory->path, scan->root->path) == 0) {
            return directory->path;
        }
    }
    return NULL;
}

/** Open the exact storage directory when present, then drain a finite FIFO.
 * Each physical namespace directory is visited once; version directories are
 * recorded rather than enqueued. [PKG-01](../../../../spec/project/packages.md#pkg-01)
 * fixes the two physical storage names independently of their logical identity.
 */
ModuleStatus package_discover(PackageScan *scan) {
    const char *storage = scan->local ? "packages" : "dependencies";
    const char *root = verified_root(scan);
    if (root == NULL) {
        return module_fail(scan->catalog, MODULE_INVALID_PATH, NULL);
    }
    const char *physical = source_arena_join(&scan->catalog->arena, root, "/", storage);
    const char *relative = source_arena_copy(&scan->catalog->arena, storage);
    if (physical == NULL || relative == NULL) {
        return module_fail(scan->catalog, MODULE_OUT_OF_MEMORY, root);
    }
    struct stat metadata;
    ModuleStatus status = scan->reader->child(scan->reader->context, scan->catalog, root, storage, physical, &metadata);
    if (status == MODULE_MISSING) {
        scan->catalog->issue = (ModuleIssue){0};
        return MODULE_OK;
    }
    if (status != MODULE_OK || !S_ISDIR(metadata.st_mode)) {
        return status != MODULE_OK ? status
            : module_fail(scan->catalog, MODULE_UNSUPPORTED_PATH, physical);
    }
    status = package_queue(scan, physical, relative, "", 0, &metadata);
    while (status == MODULE_OK && scan->pending != NULL) {
        PackageDirectory *directory = scan->pending;
        scan->pending = directory->next;
        if (scan->pending == NULL) {
            scan->last = NULL;
        }
        const char **names;
        size_t count;
        status = scan->reader->entries(scan->reader->context, scan->catalog, directory->physical, &names, &count);
        for (size_t index = 0; status == MODULE_OK && index < count; index++) {
            status = package_visit(scan, directory, names[index]);
        }
    }
    return status;
}
