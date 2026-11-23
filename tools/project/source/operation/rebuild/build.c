/** @file
 * @brief Retain complete O/U/F invalidation domains instead of guessing affected artifact filenames.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * regenerates build state normally; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires owned intentions before any publication, including root-only changes.
 */
#include "rebuild.h"
#include <stdint.h>
/** Append every explicit semantic descriptor with its phase, preserving exclusions. */
static bool append(SourceOperationRebuild *owner, const ModuleRoot *roots, size_t count, SourceOperationRebuildDomain domain) {
    for (size_t i = 0; i < count; i++) {
        SourceOperationRebuildScope *row = source_arena_allocate(&owner->arena, sizeof(*row));
        if (!row) { return false; }
        *row = (SourceOperationRebuildScope){.root = &roots[i], .domain = domain};
        if (owner->last) { owner->last->next = row; } else { owner->first = row; } owner->last = row; owner->count++;
    }
    return true;
}
SourceOperationRebuildStatus source_operation_rebuild_build(SourceOperationRebuild *owner,
    const SourceOperationRoots *roots, const SourceOperationClients *clients) {
    if (!owner || owner->attempted) { return SOURCE_OPERATION_REBUILD_INVALID; }
    owner->attempted = true; owner->status = SOURCE_OPERATION_REBUILD_INVALID;
    if (!roots || !roots->valid || !clients || !clients->valid || roots->inputs.plan != clients->plan) { return owner->status; }
    owner->roots = roots; owner->clients = clients;
    const SourceRootCapture *configured = roots->inputs.configured;
    const SourceOperationPlanInputs *inputs = &clients->plan->inputs;
    const SourceRootCapture *original = inputs->observed->original;
    if (original->root_count > SIZE_MAX - configured->root_count
        || inputs->final_root_count > SIZE_MAX - configured->root_count - original->root_count
        || configured->root_count + original->root_count + inputs->final_root_count > SIZE_MAX / sizeof(SourceOperationRebuildScope)) {
        return owner->status = SOURCE_OPERATION_REBUILD_LIMIT;
    }
    if (!append(owner, configured->roots, configured->root_count, SOURCE_OPERATION_DOMAIN_CONFIGURED)
        || !append(owner, original->roots, original->root_count, SOURCE_OPERATION_DOMAIN_ORIGINAL)
        || !append(owner, inputs->final_roots, inputs->final_root_count, SOURCE_OPERATION_DOMAIN_FINAL)) {
        return owner->status = SOURCE_OPERATION_REBUILD_MEMORY;
    }
    owner->classes = SOURCE_OPERATION_REBUILD_C | SOURCE_OPERATION_REBUILD_SOURCE_MAPS | SOURCE_OPERATION_REBUILD_NATIVE_PREFIXES
        | SOURCE_OPERATION_REBUILD_CACHES | SOURCE_OPERATION_REBUILD_TEST_OUTPUTS;
    owner->boundaries = SOURCE_OPERATION_OUTSIDE_IMPORTERS_UNCHECKED | SOURCE_OPERATION_OPAQUE_EMBEDDED_PATHS_UNREWRITTEN
        | SOURCE_OPERATION_EXTERNAL_CLIENT_FORMATS_UNWRITTEN;
    owner->valid = true; return owner->status = SOURCE_OPERATION_REBUILD_OK;
}
