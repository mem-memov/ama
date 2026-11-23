/** @file
 * @brief Recover a version directory from an exact explicit main and its decoded final root.
 * [PKG-01](../../../../../spec/project/packages.md#pkg-01) fixes physical
 * packages/dependencies and version positions; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids basename guesses or choosing a different root for added material.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <string.h>
#include <stdint.h>
/** Copy an exact bounded prefix without imposing a filename-size limit. */
static const char *prefix(SourceArena *arena, const char *text, size_t length) {
    if (length == SIZE_MAX) { return NULL; }
    char *copy = source_arena_allocate(arena, length + 1);
    if (copy) { memcpy(copy, text, length); copy[length] = 0; }
    return copy;
}
ProjectSourceSeedsStatus project_source_seeds_version(ProjectSourceSeeds *owner,
    const SourceCloneMapping *row, SourceVersionPlacement *output) {
    SourcePlacedIdentity decoded;
    SourceIdentityStatus status = source_placement_identity(&owner->arena, owner->roots, owner->root_count,
        row->path, MODULE_PATH_MODULE, &decoded);
    if (status != SOURCE_IDENTITY_OK) { return status == SOURCE_IDENTITY_MEMORY ? PROJECT_SOURCE_SEEDS_MEMORY : PROJECT_SOURCE_SEEDS_VERSION; }
    if (strcmp(decoded.identity, row->identity)) { return PROJECT_SOURCE_SEEDS_VERSION; }
    ModulePathShape shape;
    if (!module_path_describe((const unsigned char *)row->identity, strlen(row->identity), &shape)
        || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_MODULE) { return PROJECT_SOURCE_SEEDS_VERSION; }
    const char *root = owner->roots[decoded.root].path;
    const char *cursor = row->path + strlen(root);
    if (*cursor == '/') { cursor++; }
    /* One storage-space component precedes the canonical vendor/package/version
     * chain. Decoding above already validated the exact space and component roles. */
    cursor = strchr(cursor, '/');
    if (!cursor) { return PROJECT_SOURCE_SEEDS_VERSION; } cursor++;
    for (size_t i = 0; i < shape.version_components; i++) {
        const char *slash = strchr(cursor, '/');
        if (!slash) { return PROJECT_SOURCE_SEEDS_VERSION; }
        if (i + 1 == shape.version_components) {
            const char *physical = prefix(&owner->arena, row->path, (size_t)(slash - row->path));
            const char *identity = prefix(&owner->arena, row->identity, shape.version_length);
            if (!physical || !identity) { return PROJECT_SOURCE_SEEDS_MEMORY; }
            SourcePlacedIdentity version;
            status = source_placement_identity(&owner->arena, owner->roots, owner->root_count,
                physical, MODULE_PATH_VERSION, &version);
            if (status != SOURCE_IDENTITY_OK) { return status == SOURCE_IDENTITY_MEMORY ? PROJECT_SOURCE_SEEDS_MEMORY : PROJECT_SOURCE_SEEDS_VERSION; }
            if (strcmp(version.identity, identity)) { return PROJECT_SOURCE_SEEDS_VERSION; }
            *output = (SourceVersionPlacement){identity, physical}; return PROJECT_SOURCE_SEEDS_OK;
        }
        cursor = slash + 1;
    }
    return PROJECT_SOURCE_SEEDS_VERSION;
}
