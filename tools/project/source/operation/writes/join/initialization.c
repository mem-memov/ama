/** @file
 * @brief Retain only actual required new-version initialization rows with proven original absence.
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * creates empty package.ini/src/tests for wholly absent versions, never imports
 * an old configuration or repairs an existing incomplete version.
 */
#include "internal.h"
#include "../../../mapping/identity.h"
#include <string.h>
/** Actual raw/copied content already supplies this area; no extra area row is needed. */
static bool area(const SourceFileMap *files, const char *path) {
    for (const SourceFileMapping *file = files->first; file; file = file->next) {
        if (source_image_below(file->destination, path)) { return true; }
    }
    return false;
}
/** Configuration is compulsory once; area rows exist exactly when not supplied
 * by P. Symbolic identity labels are normalized by the common extra join. */
static SourceOperationWritesStatus rows(SourceOperationWrites *writes, const SourceVersionInitialization *initialization) {
    const char *suffixes[] = {"package.ini", "src", "tests"}; bool seen[3] = {0};
    for (size_t i = 0; i < initialization->count; i++) {
        const SourceImageChange *change = &initialization->changes[i]; writes->failed_path = change->entry.path;
        if (change->kind != SOURCE_IMAGE_ADD || !change->entry.path || !change->entry.metadata_captured) { return SOURCE_OPERATION_WRITES_SCOPE; }
        size_t selected = 3;
        for (size_t j = 0; j < 3; j++) {
            const char *path = source_arena_join(&writes->arena, initialization->version.physical, "/", suffixes[j]);
            if (!path) { return SOURCE_OPERATION_WRITES_MEMORY; }
            if (!strcmp(path, change->entry.path)) { selected = j; }
        }
        if (selected == 3 || seen[selected]) { return SOURCE_OPERATION_WRITES_DUPLICATE; }
        seen[selected] = true;
        if (!selected) {
            if (!S_ISREG(change->entry.metadata.st_mode) || !change->entry.content || source_length(change->entry.content)
                || strcmp(source_path(change->entry.content), source_path(initialization->configuration))
                || source_operation_writes_file(writes->inputs.files, change->entry.path)) { return SOURCE_OPERATION_WRITES_SCOPE; }
        } else if (!S_ISDIR(change->entry.metadata.st_mode) || !change->entry.children_complete || change->entry.content
            || area(writes->inputs.files, change->entry.path)) { return SOURCE_OPERATION_WRITES_SCOPE; }
        SourceOperationExtraOwner owner = {.kind = SOURCE_OPERATION_EXTRA_INITIALIZATION, .owner.initialization = initialization};
        SourceOperationWritesStatus status = source_operation_writes_extra(writes, *change, owner);
        if (status != SOURCE_OPERATION_WRITES_OK) { return status; }
    }
    if (!seen[0]) { return SOURCE_OPERATION_WRITES_SCOPE; }
    for (size_t i = 1; i < 3; i++) {
        const char *path = source_arena_join(&writes->arena, initialization->version.physical, "/", suffixes[i]);
        if (!path) { return SOURCE_OPERATION_WRITES_MEMORY; }
        writes->failed_path = path;
        if (seen[i] == area(writes->inputs.files, path)) { return SOURCE_OPERATION_WRITES_SCOPE; }
    }
    return SOURCE_OPERATION_WRITES_OK;
}
SourceOperationWritesStatus source_operation_writes_initializations(SourceOperationWrites *writes) {
    for (size_t i = 0; i < writes->inputs.initialization_count; i++) {
        const SourceVersionInitialization *initialization = writes->inputs.initializations[i];
        if (!initialization || initialization->original != writes->inputs.observed || !initialization->configuration
            || source_length(initialization->configuration) || !initialization->version.identity || !initialization->version.physical
            || !initialization->changes || !initialization->count || initialization->count > 3) { return SOURCE_OPERATION_WRITES_INVALID; }
        writes->failed_path = initialization->version.physical;
        if (!area(writes->inputs.files, initialization->version.physical)) { return SOURCE_OPERATION_WRITES_SCOPE; }
        for (size_t j = 0; j < i; j++) {
            const SourceVersionInitialization *old = writes->inputs.initializations[j];
            if (!strcmp(old->version.identity, initialization->version.identity)
                || !strcmp(old->version.physical, initialization->version.physical)) { return SOURCE_OPERATION_WRITES_DUPLICATE; }
        }
        if (!source_image_path_absent(&writes->inputs.observed->image, initialization->version.physical)) { return SOURCE_OPERATION_WRITES_SCOPE; }
        SourcePlacedIdentity placed;
        SourceIdentityStatus identity = source_placement_identity(&writes->arena, initialization->final_roots,
            initialization->final_root_count, initialization->version.physical, MODULE_PATH_VERSION, &placed);
        if (identity == SOURCE_IDENTITY_MEMORY) { return SOURCE_OPERATION_WRITES_MEMORY; }
        if (identity != SOURCE_IDENTITY_OK || strcmp(placed.identity, initialization->version.identity)) { return SOURCE_OPERATION_WRITES_SCOPE; }
        const char *configuration = source_arena_join(&writes->arena, initialization->version.physical, "/", "package.ini");
        if (!configuration) { return SOURCE_OPERATION_WRITES_MEMORY; }
        if (strcmp(configuration, source_path(initialization->configuration))) { return SOURCE_OPERATION_WRITES_SCOPE; }
        SourceOperationWritesStatus status = rows(writes, initialization);
        if (status != SOURCE_OPERATION_WRITES_OK) { return status; }
    }
    return SOURCE_OPERATION_WRITES_OK;
}
