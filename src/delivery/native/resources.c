/** @file
 * @brief Walk the explicitly owned native resource unit without interpreting CMake.
 * [PKG-07](../../../spec/project/packages.md#pkg-07) supplies exact ownership;
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01)
 * forbids heuristic neighboring-file recovery and generation-time CMake execution.
 * [COMP-05](../../../spec/implementation/compiler.md#comp-05) requires stable order.
 * [SRC-02](../../../spec/language/source-and-names.md#src-02) rejects repeated
 * physical directory identities, bounding traversal even in aliased mount trees.
 */
#include <string.h>
#include "resources.h"

/** Arena-owned pending directory; each physical child is visited once without
 * following links under [PKG-07](../../../spec/project/packages.md#pkg-07).
 */
typedef struct Pending {
    struct Pending *next;
    const char *path;
} Pending;

/** Preserve each complete path once in byte order under
 * [COMP-05](../../../spec/implementation/compiler.md#comp-05).
 */
static void insert(DeliveryNativeResource **first, DeliveryNativeResource *row) {
    while (*first != NULL && strcmp((*first)->file.path, row->file.path) < 0) { first = &(*first)->next; }
    row->next = *first; *first = row;
}

/** Known core inputs retain their authoritative compiler snapshots instead of
 * being reread as assets under [BUILD-01](../../../spec/implementation/application-builds.md#build-01).
 */
static bool core(const char *suffix) {
    return !strcmp(suffix, ".c") || !strcmp(suffix, ".h") || !strcmp(suffix, ".abi")
        || !strcmp(suffix, ".ini") || !strcmp(suffix, ".cmake");
}

/** Observe one selected child. An absent subtree root is explicit; an enumerated
 * entry disappearing during a stable-input capture is an I/O failure under
 * [PKG-07](../../../spec/project/packages.md#pkg-07). Directory queue nodes retain
 * only copied paths; every resource byte belongs to the supplied SourceManager.
 */
static SourceStatus child(SourceArena *arena, SourceManager *texts, const char *parent,
    const char *name, bool root, ModuleCatalog *entries, DeliveryNativeResource **first,
    Pending ***tail, const char **failed) {
    const char *path = source_arena_join(arena, parent, parent[strlen(parent) - 1] == '/' ? "" : "/", name);
    if (path == NULL) { return SOURCE_OUT_OF_MEMORY; }
    *failed = path;
    struct stat metadata;
    PathProbe probe = path_exact_child(parent, name, path, &metadata);
    if (probe != PATH_FOUND && !(root && probe == PATH_MISSING)) { return SOURCE_IO_ERROR; }
    if (probe == PATH_FOUND && !S_ISREG(metadata.st_mode) && !S_ISDIR(metadata.st_mode)) { return SOURCE_IO_ERROR; }
    if (root && probe == PATH_FOUND && !S_ISDIR(metadata.st_mode)) { return SOURCE_IO_ERROR; }
    DeliveryNativeResource *row = source_arena_allocate(arena, sizeof(*row));
    if (row == NULL) { return SOURCE_OUT_OF_MEMORY; }
    *row = (DeliveryNativeResource){.file = {.path = path, .issue = probe,
        .state = probe == PATH_MISSING ? DELIVERY_FILE_ABSENT : DELIVERY_FILE_PRESENT},
        .directory = root || (probe == PATH_FOUND && S_ISDIR(metadata.st_mode))};
    if (probe == PATH_FOUND && row->directory) {
        ModuleStatus remembered = module_remember_directory_metadata(entries, path, &metadata);
        if (remembered != MODULE_OK) { return remembered == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR; }
        Pending *pending = source_arena_allocate(arena, sizeof(*pending));
        if (pending == NULL) { return SOURCE_OUT_OF_MEMORY; }
        *pending = (Pending){.path = path}; **tail = pending; *tail = &pending->next;
    } else if (probe == PATH_FOUND) {
        SourceStatus status = delivery_file_capture(arena, texts, parent, name, &row->file);
        if (status != SOURCE_OK) { return status; }
        if (row->file.state != DELIVERY_FILE_PRESENT) { return SOURCE_IO_ERROR; }
    }
    insert(first, row);
    return SOURCE_OK;
}

SourceStatus delivery_native_resources(SourceArena *arena, SourceManager *texts, const ModuleSource *source,
    const DeliveryNativeResource **output, const char **failed_path) {
    if (arena == NULL || texts == NULL || source == NULL || output == NULL || failed_path == NULL) {
        return SOURCE_INVALID_ARGUMENT;
    }
    char *parent = source_arena_copy(arena, source_path(source->text));
    if (parent == NULL) { return SOURCE_OUT_OF_MEMORY; }
    char *slash = strrchr(parent, '/');
    const char *basename = slash == NULL ? parent : slash + 1;
    const char *prefix = source_arena_join(arena, basename, "", ".");
    const char *tree = source_arena_join(arena, basename, "", "+");
    if (prefix == NULL || tree == NULL) { return SOURCE_OUT_OF_MEMORY; }
    if (slash != NULL) { *slash = 0; }
    const char *directory = slash == NULL ? "." : (*parent == 0 ? "/" : parent);
    size_t prefix_length = strlen(prefix);
    Pending *pending = NULL, **tail = &pending;
    DeliveryNativeResource *first = NULL;
    ModuleCatalog entries; module_catalog_init(&entries, arena->allocator);
    const char **names = NULL; size_t count = 0;
    ModuleStatus listed = module_catalog_entries(&entries, directory, &names, &count);
    SourceStatus status = listed == MODULE_OK ? SOURCE_OK
        : listed == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR;
    if (status != SOURCE_OK) { *failed_path = directory; }
    for (size_t index = 0; status == SOURCE_OK && index < count; index++) {
        const char *name = names[index];
        if (strncmp(name, prefix, prefix_length) || name[prefix_length] == 0 || core(name + prefix_length - 1)) { continue; }
        const char *path = source_arena_join(arena, directory,
            directory[strlen(directory) - 1] == '/' ? "" : "/", name);
        if (path == NULL) { status = SOURCE_OUT_OF_MEMORY; break; }
        struct stat metadata;
        if (path_exact_child(directory, name, path, &metadata) != PATH_FOUND || !S_ISREG(metadata.st_mode)) {
            *failed_path = path; status = SOURCE_IO_ERROR; break;
        }
        status = child(arena, texts, directory, name, false, &entries, &first, &tail, failed_path);
    }
    if (status == SOURCE_OK) { status = child(arena, texts, directory, tree, true, &entries, &first, &tail, failed_path); }
    while (status == SOURCE_OK && pending != NULL) {
        const char *current = pending->path;
        pending = pending->next;
        if (pending == NULL) { tail = &pending; }
        listed = module_catalog_entries(&entries, current, &names, &count);
        if (listed != MODULE_OK) {
            *failed_path = current;
            status = listed == MODULE_OUT_OF_MEMORY ? SOURCE_OUT_OF_MEMORY : SOURCE_IO_ERROR;
            break;
        }
        for (size_t index = 0; status == SOURCE_OK && index < count; index++) {
            status = child(arena, texts, current, names[index], false, &entries, &first, &tail, failed_path);
        }
    }
    module_catalog_destroy(&entries);
    if (status == SOURCE_OK) { *output = first; *failed_path = NULL; }
    return status;
}
