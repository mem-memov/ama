/** @file
 * @brief Derive exact explicit seeds and shared logical destinations from admitted command originals.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids a command-specific binding solver or discovery-order destination choice.
 */
#include "internal.h"
ProjectSourceSeedsStatus project_source_seeds_build(ProjectSourceSeeds *owner, const SourceTreePlacement *placement,
    SourceAnalysis *analysis, const SourceDeclaredGroup *declared, SourceCopyDependencyMode mode,
    const ModuleRoot *roots, size_t count) {
    if (!owner || owner->attempted) { return PROJECT_SOURCE_SEEDS_INVALID; }
    owner->attempted = true;
    if (!placement || !placement->ready || !placement->selection || !placement->selection->valid
        || !analysis || !analysis->valid
        || (count && !roots) || (mode != SOURCE_COPY_KEEP_EXTERNAL && mode != SOURCE_COPY_EXTEND_PACKAGE
            && mode != SOURCE_COPY_CLONE_DEPENDENCIES)) { return owner->status = PROJECT_SOURCE_SEEDS_INVALID; }
    owner->placement = placement; owner->roots = roots; owner->root_count = count;
    /* These initially empty child owners receive their actual borrowed original
     * domain before their sole build; no success/readiness flag is manufactured. */
    owner->copy.expansion.capture = placement->selection->capture;
    owner->copy.expansion.analysis = analysis; owner->copy.expansion.declared = declared;
    owner->explicit_mains.capture = placement->selection->capture;
    if (source_clone_map_propose_tree(&owner->explicit_mains, placement, roots, count,
        &owner->explicit_issue) != SOURCE_CLONE_OK) {
        owner->failed_path = owner->explicit_issue.path; return owner->status = PROJECT_SOURCE_SEEDS_EXPLICIT;
    }
    if (source_clone_map_seal(&owner->explicit_mains) != SOURCE_CLONE_OK) { return owner->status = PROJECT_SOURCE_SEEDS_EXPLICIT; }
    if (source_copy_closure_build(&owner->copy, placement->selection, mode) != SOURCE_COPY_CLOSURE_OK) {
        owner->failed_path = owner->copy.failed_path; return owner->status = PROJECT_SOURCE_SEEDS_CLOSURE;
    }
    if (source_copy_destinations_build(&owner->destinations, &owner->copy, &owner->explicit_mains) != SOURCE_COPY_DESTINATIONS_OK) {
        return owner->status = PROJECT_SOURCE_SEEDS_DESTINATIONS;
    }
    owner->status = project_source_seeds_versions(owner);
    if (owner->status == PROJECT_SOURCE_SEEDS_OK) { owner->valid = true; }
    return owner->status;
}
