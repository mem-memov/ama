/** @file
 * @brief Prepare whole selected versions without inventing executable module roots.
 * [PKG-07](../../../../spec/project/packages.md#pkg-07) retains complete source
 * trees; [PKG-08](../../../../spec/project/packages.md#pkg-08) applies tests only
 * to actual active source modules, including no invented tests for an empty version.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
/** A selected version's catalog backlink and canonical area prefix determine membership. */
static bool member(const ModuleSource *source, const ModuleVersion *version) {
    size_t length = strlen(version->logical_path);
    return source->version == version && !strncmp(source->logical_path, version->logical_path, length)
        && !strncmp(source->logical_path + length, "/src/", 5);
}
AcquisitionClosureStatus source_expansion_scope(SourceExpansion *expansion,
    const ModuleVersion *version, AcquisitionExpansion *fact) {
    AcquisitionClosureStatus status = source_expansion_version(expansion, version, fact);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    expansion->file_status = acquisition_directory_capture(&expansion->arena, &expansion->files,
        version->physical_path, "src", &fact->files, &expansion->failed_path);
    if (expansion->file_status != SOURCE_OK) {
        return expansion->file_status == SOURCE_OUT_OF_MEMORY || expansion->file_status == SOURCE_LIMIT
            ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_PREPARATION_FAILED;
    }
    const char *directory = source_arena_join(&expansion->arena, version->physical_path, "/", "src");
    if (!directory) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    status = source_expansion_observed(expansion, fact->files, directory, false);
    if (status != ACQUISITION_CLOSURE_OK) { return status; }
    size_t count = 0;
    for (const ModuleSource *source = expansion->capture->catalog.first; source; source = source->next) {
        if (member(source, version)) {
            if (count == SIZE_MAX) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
            count++;
        }
    }
    if (count > SIZE_MAX / sizeof(const char *)) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    const char **modules = count ? source_arena_allocate(&expansion->arena, count * sizeof(*modules)) : NULL;
    if (count && !modules) { return ACQUISITION_CLOSURE_OUT_OF_MEMORY; }
    size_t index = 0;
    for (const ModuleSource *source = expansion->capture->catalog.first; source; source = source->next) {
        if (member(source, version)) { modules[index++] = source->logical_path; }
    }
    fact->submodules = modules;
    fact->submodule_count = count;
    return ACQUISITION_CLOSURE_OK;
}
