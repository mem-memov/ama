/** @file
 * @brief Verify the exact same-path analysis-scope union law on a finite subtree algebra.
 * @see [T287](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
#include "../../../../../../tools/project/command/source/augmentation/scopes.h"
/** The independent bit model supplies ancestor membership for nested and
 * byte-prefix-only siblings; enumerate all pairs of excluded-subtree unions. */
static void it_unions_equal_root_scopes_by_the_exclusion_intersection_law(void) {
    setup_test();
    const char *names[] = {"a", "a/b", "ab", "c"};
    const unsigned points[] = {1, 3, 3, 4, 4, 8, 8, 0};
    size_t baseline = live_allocations;
    for (unsigned a = 0; a < 16; a++) {
        for (unsigned b = 0; b < 16; b++) {
            const char *first[4], *second[4]; size_t ac = 0, bc = 0;
            for (unsigned i = 0; i < 4; i++) { if (a & (1u << i)) { first[ac++] = names[i]; } if (b & (1u << i)) { second[bc++] = names[i]; } }
            ModuleRoot target = {"/root", first, ac, MODULE_ROOT_STANDARD};
            ModuleRoot added = {"/root", second, bc, MODULE_ROOT_STANDARD};
            SourceArena arena; source_arena_init(&arena, sources.allocator);
            assert(project_source_augmentation_scope_union(&arena, &target, &added) == PROJECT_SOURCE_AUGMENTATION_OK);
            unsigned excluded = 0;
            for (size_t i = 0; i < target.excluded_count; i++) {
                unsigned found = 0;
                for (unsigned j = 0; j < 4; j++) { if (!strcmp(target.excluded[i], names[j])) { found = 1u << j; } }
                assert(found && !(excluded & found)); excluded |= found;
            }
            for (size_t p = 0; p < sizeof(points)/sizeof(*points); p++) {
                bool expected = !(a & points[p]) || !(b & points[p]);
                assert((!(excluded & points[p])) == expected);
            }
            source_arena_destroy(&arena); assert(live_allocations == baseline);
        }
    }
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    ModuleRoot first = {"/root", names, 4, MODULE_ROOT_STANDARD}, second = first;
    failure_at = allocation_count + 1;
    assert(project_source_augmentation_scope_union(&arena, &first, &second) == PROJECT_SOURCE_AUGMENTATION_MEMORY
        && first.excluded == names && first.excluded_count == 4);
    failure_at = 0;
    second.path = NULL;
    assert(project_source_augmentation_scope_union(&arena, &first, &second) == PROJECT_SOURCE_AUGMENTATION_INVALID);
    source_arena_destroy(&arena); assert(live_allocations == baseline);
    destroy_test();
}
