/** @file
 * @brief Construct actual copy main rows and reject explicit physical/topology disagreement.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * defines one injective map consistent with P; [SRC-05](../../../../../spec/language/source-and-names.md#src-05)
 * requires final-name-driven physical plus boundaries.
 */
#include "physical.h"
#include "source/path/logical.h"
#include <string.h>
/** Match complete canonical version boundaries, never textual package guesses. */
static bool belongs(const char *identity, const char *version) {
    size_t length = strlen(version), total = strlen(identity);
    return total > length && identity[length] == '/' && !memcmp(identity, version, length);
}
/** A failed preparation prefix must never become a sealed clone relation. */
static SourceCopyMainsStatus fail(SourceCloneMap *map, SourceCopyMainsStatus status) {
    map->status = status == SOURCE_COPY_MAINS_MEMORY ? SOURCE_CLONE_MEMORY : SOURCE_CLONE_INVALID; return status;
}
/** Add one actual row only after validating its complete final namespace and P. */
static SourceCopyMainsStatus place(SourceCloneMap *map, const ModuleSource *original, const char *identity,
    const SourceCloneMapping *explicit_row, const SourceVersionPlacement *version,
    const char *const *mains, size_t count, SourceCopyMainsIssue *issue) {
    *issue = (SourceCopyMainsIssue){.original = original, .explicit_row = explicit_row};
    issue->path_status = source_main_physical_path(&map->arena, version, mains, count, identity, &issue->expected);
    if (issue->path_status != SOURCE_MAIN_PATH_OK) {
        return fail(map, issue->path_status == SOURCE_MAIN_PATH_MEMORY ? SOURCE_COPY_MAINS_MEMORY : SOURCE_COPY_MAINS_TOPOLOGY);
    }
    if (explicit_row && strcmp(explicit_row->path, issue->expected)) { return fail(map, SOURCE_COPY_MAINS_EXPLICIT); }
    const SourceCloneMapping *row = NULL;
    SourceCloneStatus status = source_clone_map_add(map, original, issue->expected, identity, &row);
    return status == SOURCE_CLONE_OK ? SOURCE_COPY_MAINS_OK :
        status == SOURCE_CLONE_MEMORY ? SOURCE_COPY_MAINS_MEMORY : SOURCE_COPY_MAINS_MAP;
}
SourceCopyMainsStatus source_copy_place_version_mains(SourceCloneMap *map, const SourceCopyDestinations *result,
    const SourceVersionPlacement *version, const char *const *mains, size_t count, SourceCopyMainsIssue *issue) {
    if (!issue) { return SOURCE_COPY_MAINS_INVALID; }
    *issue = (SourceCopyMainsIssue){0};
    if (!map || map->ready || map->status != SOURCE_CLONE_OK) { return SOURCE_COPY_MAINS_INVALID; }
    if (result && map == result->explicit_map) { return SOURCE_COPY_MAINS_INVALID; }
    ModulePathShape shape;
    if (!result || !result->valid || !result->explicit_map ||
        map->capture != result->explicit_map->capture || !version || !version->identity ||
        !version->physical || version->physical[0] != '/' || (count && !mains) ||
        !module_path_describe((const unsigned char *)version->identity, strlen(version->identity), &shape) ||
        shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) { return fail(map, SOURCE_COPY_MAINS_INVALID); }
    for (const SourceCloneMapping *row = result->explicit_map->first; row; row = row->next) {
        if (!belongs(row->identity, version->identity)) { continue; }
        SourceCopyMainsStatus status = place(map, row->original, row->identity, row, version, mains, count, issue);
        if (status != SOURCE_COPY_MAINS_OK) { return status; }
    }
    for (size_t i = 0; i < result->count; i++) {
        const SourceCopyDestination *entry = &result->entries[i];
        if (strcmp(entry->version, version->identity)) { continue; }
        const SourceCloneMapping *explicit_row = result->explicit_map->first;
        while (explicit_row && explicit_row->original != entry->source) { explicit_row = explicit_row->next; }
        SourceCopyMainsStatus status = place(map, entry->source, entry->identity, explicit_row, version, mains, count, issue);
        if (status != SOURCE_COPY_MAINS_OK) { return status; }
    }
    *issue = (SourceCopyMainsIssue){0}; return SOURCE_COPY_MAINS_OK;
}
