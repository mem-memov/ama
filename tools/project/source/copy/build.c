/** @file
 * @brief Admit exact raw selection and original proof before one finite copy-closure computation.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes files/directories; [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * uses actually indexed source seeds and explicit dependency modes, never logical
 * prefix replacement or reachability guesses. No result authorizes publication.
 */
#include <stdint.h>
#include "internal.h"
SourceCopyClosureStatus source_copy_closure_build(SourceCopyClosure *copy,
    const SourceTreeSelection *tree, SourceCopyDependencyMode mode) {
    if (!copy || copy->attempted) { return SOURCE_COPY_CLOSURE_INVALID; }
    copy->attempted = true;
    if (!tree || !tree->valid || !tree->capture || !tree->capture->valid
        || tree->capture != copy->expansion.capture
        || (mode != SOURCE_COPY_KEEP_EXTERNAL && mode != SOURCE_COPY_EXTEND_PACKAGE && mode != SOURCE_COPY_CLONE_DEPENDENCIES)) {
        return copy->status = SOURCE_COPY_CLOSURE_INVALID;
    }
    copy->tree = tree; copy->mode = mode;
    copy->domain_status = source_tree_selection_domain(tree, &copy->failed_path);
    if (copy->domain_status != SOURCE_TREE_DOMAIN_OK) { return copy->status = SOURCE_COPY_CLOSURE_DOMAIN; }
    copy->preparation_status = source_expansion_validate(&copy->expansion, &copy->issue);
    if (copy->preparation_status != ACQUISITION_CLOSURE_OK) {
        copy->failed_path = copy->expansion.failed_path;
        return copy->status = SOURCE_COPY_CLOSURE_PREPARATION;
    }
    if (tree->count > SIZE_MAX / sizeof(const char *)) { return copy->status = SOURCE_COPY_CLOSURE_MEMORY; }
    const char **seeds = tree->count ? source_arena_allocate(&copy->arena, tree->count * sizeof(*seeds)) : NULL;
    if (tree->count && !seeds) { return copy->status = SOURCE_COPY_CLOSURE_MEMORY; }
    for (size_t index = 0; index < tree->count; index++) { seeds[index] = tree->units[index]->source->logical_path; }
    copy->seeds = seeds; copy->seed_count = tree->count;
    AcquisitionClosureProvider provider = source_copy_closure_callbacks(copy);
    copy->expansion_status = acquisition_closure_compute(&copy->closure, seeds, tree->count, &provider);
    if (copy->expansion_status != ACQUISITION_CLOSURE_OK) {
        copy->issue = copy->closure.issue; copy->failed_path = copy->expansion.failed_path;
        return copy->status = SOURCE_COPY_CLOSURE_EXPANSION;
    }
    copy->valid = true;
    return copy->status = SOURCE_COPY_CLOSURE_OK;
}
