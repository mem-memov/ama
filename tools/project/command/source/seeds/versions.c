/** @file
 * @brief Require one actual explicit storage authority per designated ordinary destination version.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires an explicit cross-version policy; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * propagates the selected unit's designated version without choosing by discovery order.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <stdint.h>
#include <string.h>
/** Exact version identity determines the authority; equal repeats coalesce. */
static ProjectSourceSeedsStatus add(ProjectSourceSeeds *owner, SourceVersionPlacement version) {
    for (size_t i = 0; i < owner->version_count; i++) {
        if (strcmp(owner->versions[i].identity, version.identity)) { continue; }
        return strcmp(owner->versions[i].physical, version.physical) ? PROJECT_SOURCE_SEEDS_VERSION : PROJECT_SOURCE_SEEDS_OK;
    }
    owner->versions[owner->version_count++] = version; return PROJECT_SOURCE_SEEDS_OK;
}
ProjectSourceSeedsStatus project_source_seeds_versions(ProjectSourceSeeds *owner) {
    size_t capacity = 0;
    for (const SourceCloneMapping *row = owner->explicit_mains.first; row; row = row->next) {
        if (!strncmp(row->identity, "//", 2)) { continue; }
        if (capacity == SIZE_MAX) { return PROJECT_SOURCE_SEEDS_MEMORY; } capacity++;
    }
    if (capacity > SIZE_MAX / sizeof(*owner->versions)) { return PROJECT_SOURCE_SEEDS_MEMORY; }
    owner->versions = capacity ? source_arena_allocate(&owner->arena, capacity * sizeof(*owner->versions)) : NULL;
    if (capacity && !owner->versions) { return PROJECT_SOURCE_SEEDS_MEMORY; }
    for (const SourceCloneMapping *row = owner->explicit_mains.first; row; row = row->next) {
        if (!strncmp(row->identity, "//", 2)) { continue; }
        owner->failed_row = row; owner->failed_path = row->path;
        SourceVersionPlacement version;
        ProjectSourceSeedsStatus status = project_source_seeds_version(owner, row, &version);
        if (status != PROJECT_SOURCE_SEEDS_OK) { return status; }
        if (owner->copy.mode == SOURCE_COPY_KEEP_EXTERNAL
            && strcmp(row->original->version->logical_path, version.identity)) { return PROJECT_SOURCE_SEEDS_POLICY; }
        status = add(owner, version); if (status != PROJECT_SOURCE_SEEDS_OK) { return status; }
    }
    for (size_t i = 0; i < owner->destinations.count; i++) {
        const char *version = owner->destinations.entries[i].version; bool found = false;
        for (size_t j = 0; j < owner->version_count; j++) { found |= !strcmp(owner->versions[j].identity, version); }
        if (!found) { owner->failed_path = version; return PROJECT_SOURCE_SEEDS_VERSION; }
    }
    owner->failed_row = NULL; owner->failed_path = NULL; return PROJECT_SOURCE_SEEDS_OK;
}
