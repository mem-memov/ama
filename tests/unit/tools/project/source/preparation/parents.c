/** @file
 * @brief Check each positional parent-planner allocation failure against actual immutable originals.
 * @see [T290](../../../../../../docs/implementation/source-operations.md#t290-command-relation-construction-after-t292)
 */
#include "../../../../../../tools/project/source/copy/contexts/placement/placement.h"
/** Rebuild only candidate C metadata for each injected failure. Original compiler,
 * closure and observation owners are shared immutable facts, not recreated proofs.
 * Failed prefixes must poison sealing and release all arena allocations. */
static void check_context_parent_failures(ProjectSourcePreparation *original, const SourceCopyClosure *copy,
    const SourceObservedImage *observed) {
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, original->original_capture);
        SourceCloneTreeIssue seed;
        assert(source_clone_map_propose_tree(&mains, &original->placement, original->root_settings.final,
            original->root_settings.final_count, &seed) == SOURCE_CLONE_OK);
        SourceCopyParentInputs inputs = {.copy = copy, .mains = &mains, .observed = observed,
            .roots = original->root_settings.final, .root_count = original->root_settings.final_count};
        SourceCopyParentIssue issue; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceCopyParentStatus status = source_copy_context_place(&inputs, &issue);
        if (!failure) { assert(status == SOURCE_COPY_PARENT_OK && !issue.path); total = allocation_count - before; assert(total); }
        else {
            assert(status == SOURCE_COPY_PARENT_MEMORY && !mains.ready && issue.requester);
            assert(mains.status == SOURCE_CLONE_MEMORY);
        }
        failure_at = 0;
        assert(source_clone_map_seal(&mains) == (failure ? SOURCE_CLONE_MEMORY : SOURCE_CLONE_OK));
        before = allocation_count;
        assert(source_copy_context_place(&inputs, &issue) == SOURCE_COPY_PARENT_INVALID);
        assert(allocation_count == before);
        source_clone_map_destroy(&mains); assert(live_allocations == baseline);
    }
    SourceCopyParentIssue issue;
    assert(source_copy_context_place(NULL, &issue) == SOURCE_COPY_PARENT_INVALID);
    assert(source_copy_context_place(NULL, NULL) == SOURCE_COPY_PARENT_INVALID);
}
