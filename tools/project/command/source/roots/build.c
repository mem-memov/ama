/** @file
 * @brief Bound and compose configured-root dispositions, actual carries and explicit destination settings.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps finite proposals distinct from the final operation certificate.
 */
#include "internal.h"
ProjectSourceRootSettingsStatus project_source_root_settings_build(ProjectSourceRootSettings *owner,
    const SourceRootCapture *configured, const SourceTreePlacement *placement, SourceOperationKind kind,
    const ProjectSourceRootRequest *extras, size_t count) {
    if (!owner || owner->attempted) { return PROJECT_SOURCE_ROOTS_INVALID; }
    owner->attempted = true;
    if (!configured || !configured->valid || !placement || !placement->ready
        || placement->selection->capture != configured || (count && !extras)
        || (kind != SOURCE_OPERATION_COPY && kind != SOURCE_OPERATION_MOVE)) { return owner->status = PROJECT_SOURCE_ROOTS_INVALID; }
    owner->configured = configured; owner->placement = placement;
    size_t original = configured->root_count;
    if (count > SIZE_MAX / sizeof(ProjectSourceRootRequest) || original > (SIZE_MAX - count) / 2) { return owner->status = PROJECT_SOURCE_ROOTS_LIMIT; }
    owner->capacity = 2 * original + count;
    if (owner->capacity > SIZE_MAX / sizeof(ModuleRoot) || owner->capacity > SIZE_MAX / sizeof(SourceOperationRootAddition)
        || original > SIZE_MAX / sizeof(SourceOperationRootDisposition)) { return owner->status = PROJECT_SOURCE_ROOTS_LIMIT; }
    owner->final = owner->capacity ? source_arena_allocate(&owner->arena, owner->capacity * sizeof(*owner->final)) : NULL;
    owner->additions = owner->capacity ? source_arena_allocate(&owner->arena, owner->capacity * sizeof(*owner->additions)) : NULL;
    owner->originals = original ? source_arena_allocate(&owner->arena, original * sizeof(*owner->originals)) : NULL;
    if ((owner->capacity && (!owner->final || !owner->additions)) || (original && !owner->originals)) { return owner->status = PROJECT_SOURCE_ROOTS_MEMORY; }
    owner->status = project_source_root_settings_originals(owner, kind);
    if (owner->status == PROJECT_SOURCE_ROOTS_OK) { owner->status = project_source_root_settings_explicit(owner, extras, count); }
    owner->valid = owner->status == PROJECT_SOURCE_ROOTS_OK; return owner->status;
}
