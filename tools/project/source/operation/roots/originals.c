/** @file
 * @brief Preserve already covered names and complete bytes while augmenting original analysis roots.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps original identities and performs whole-version selection in one augmented
 * catalog; a physical observer is not permission to reinterpret old sources.
 */
#include "internal.h"
#include "../../capture/preservation.h"
#include <string.h>
/** Compare actual captured configuration bytes against immutable original observations. */
static bool same_configuration(const SourceCatalogImage *image, const SourceText *configuration) {
    for (size_t i = 0; i < image->count; i++) {
        const SourceCatalogImageEntry *entry = &image->entries[i];
        if (!strcmp(entry->path, source_path(configuration))) {
            return entry->metadata_captured && S_ISREG(entry->metadata.st_mode) && entry->content
                && source_length(entry->content) == source_length(configuration)
                && !memcmp(source_bytes(entry->content), source_bytes(configuration), source_length(configuration));
        }
    }
    return false;
}
/** The shared structural decoder preserves root kind and exact path components;
 * it does not incorrectly require a previously selected supplier to stay selected. */
static SourceOperationRootsStatus identity(SourceOperationRoots *owner, const char *path,
    const char *name, ModulePathExtent extent) {
    const SourceRootCapture *augmented = owner->inputs.plan->inputs.observed->original;
    SourcePlacedIdentity actual; owner->failed_path = path;
    owner->identity_status = source_placement_identity(&owner->arena, augmented->roots, augmented->root_count, path, extent, &actual);
    if (owner->identity_status == SOURCE_IDENTITY_MEMORY) { return SOURCE_OPERATION_ROOTS_MEMORY; }
    return owner->identity_status == SOURCE_IDENTITY_OK && !strcmp(actual.identity, name)
        ? SOURCE_OPERATION_ROOTS_OK : SOURCE_OPERATION_ROOTS_IDENTITY;
}
SourceOperationRootsStatus source_operation_roots_originals(SourceOperationRoots *owner) {
    const SourceRootCapture *configured = owner->inputs.configured;
    const SourceObservedImage *observed = owner->inputs.plan->inputs.observed;
    if (configured == observed->original) { return SOURCE_OPERATION_ROOTS_OK; }
    for (const SourceCapturedUnit *unit = configured->units; unit; unit = unit->next) {
        const char *path = source_path(unit->source->text); owner->failed_path = path;
        if (!source_captured_unit_preserved(unit, &observed->image)) { return SOURCE_OPERATION_ROOTS_ORIGINAL; }
        SourceOperationRootsStatus status = identity(owner, path, unit->source->logical_path, MODULE_PATH_MODULE);
        if (status != SOURCE_OPERATION_ROOTS_OK) { return status; }
    }
    for (const ModuleVersion *version = configured->catalog.versions; version; version = version->next) {
        owner->failed_path = source_path(version->configuration);
        if (!same_configuration(&observed->image, version->configuration)) { return SOURCE_OPERATION_ROOTS_ORIGINAL; }
        SourceOperationRootsStatus status = identity(owner, version->physical_path, version->logical_path, MODULE_PATH_VERSION);
        if (status != SOURCE_OPERATION_ROOTS_OK) { return status; }
    }
    owner->failed_path = NULL; return SOURCE_OPERATION_ROOTS_OK;
}
