/** @file
 * @brief Exhaust symbolic root coverage against an independent finite tree-set model.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/roots/internal.h"
/** Heap-numbered ternary ancestry is independent of production path-prefix logic. */
static bool root_model_below(size_t node, size_t ancestor) {
    while (node > ancestor) { node = (node - 1) / 3; }
    return node == ancestor;
}
/** Count-free bitsets describe all 40 points through depth three. Every model
 * boundary ends by depth two; the third level represents unchanged continuation
 * regions, including an unrelated 'z' branch. No compiler/host owner is mocked. */
static uint64_t root_model_set(size_t root, size_t first, size_t second) {
    uint64_t bits = 0;
    for (size_t point = 0; point < 40; point++) {
        if (root_model_below(point, root) && (first == SIZE_MAX || !root_model_below(point, first))
            && (second == SIZE_MAX || !root_model_below(point, second))) { bits |= UINT64_C(1) << point; }
    }
    return bits;
}
/** All 36 single/two-exclusion scopes against all pairs of providers: 46,656
 * exact inclusions, plus missing/wrong-family and allocation-prefix boundaries. */
static void it_decides_root_region_unions_against_exhaustive_tree_sets(void) {
    setup_test(); char paths[40][64]; const char *names[] = {"a", "b", "z"};
    strcpy(paths[0], "/root");
    for (size_t i = 1; i < 40; i++) {
        size_t parent = (i - 1) / 3, length = strlen(paths[parent]);
        assert(length + 3 < sizeof(paths[i])); memcpy(paths[i], paths[parent], length);
        paths[i][length] = '/'; strcpy(paths[i] + length + 1, names[(i - 1) % 3]);
    }
    ModuleRoot patterns[36]; const char *excluded[36][2]; uint64_t masks[36]; size_t used = 0;
    for (size_t root = 0; root < 13; root++) {
        patterns[used] = (ModuleRoot){paths[root], NULL, 0, MODULE_ROOT_PROJECT};
        masks[used++] = root_model_set(root, SIZE_MAX, SIZE_MAX);
        for (size_t omission = root + 1; omission < 13; omission++) {
            if (!root_model_below(omission, root)) { continue; }
            excluded[used][0] = paths[omission] + strlen(paths[root]) + 1;
            patterns[used] = (ModuleRoot){paths[root], excluded[used], 1, MODULE_ROOT_PROJECT};
            masks[used++] = root_model_set(root, omission, SIZE_MAX);
        }
    }
    assert(used == 34);
    for (size_t second = 2; second <= 4; second += 2) {
        excluded[used][0] = paths[1] + strlen(paths[0]) + 1;
        excluded[used][1] = paths[second] + strlen(paths[0]) + 1;
        patterns[used] = (ModuleRoot){paths[0], excluded[used], 2, MODULE_ROOT_PROJECT};
        masks[used++] = root_model_set(0, 1, second);
    }
    assert(used == 36);
    for (size_t wanted = 0; wanted < used; wanted++) {
        for (size_t left = 0; left < used; left++) {
            for (size_t right = 0; right < used; right++) {
                ModuleRoot providers[] = {patterns[left], patterns[right]};
                SourceArena scratch; source_arena_init(&scratch, sources.allocator); const char *witness = NULL;
                SourceOperationRootsStatus expected = (masks[wanted] & ~(masks[left] | masks[right]))
                    ? SOURCE_OPERATION_ROOTS_COVERAGE : SOURCE_OPERATION_ROOTS_OK;
                assert(source_operation_root_region(&scratch, &patterns[wanted], providers, 2, &witness) == expected);
                assert((witness != NULL) == (expected == SOURCE_OPERATION_ROOTS_COVERAGE)); source_arena_destroy(&scratch);
            }
        }
    }
    ModuleRoot wanted = {paths[0], NULL, 0, MODULE_ROOT_PROJECT};
    const char *skip[] = {"a"};
    ModuleRoot providers[] = {{paths[0], skip, 1, MODULE_ROOT_PROJECT}, {paths[1], NULL, 0, MODULE_ROOT_PROJECT}};
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena scratch; source_arena_init(&scratch, sources.allocator); const char *witness = NULL;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceOperationRootsStatus status = source_operation_root_region(&scratch, &wanted, providers, 2, &witness);
        if (!failure) { assert(status == SOURCE_OPERATION_ROOTS_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_OPERATION_ROOTS_MEMORY); }
        failure_at = 0; source_arena_destroy(&scratch); assert(live_allocations == baseline);
    }
    SourceArena scratch; source_arena_init(&scratch, sources.allocator); const char *witness = NULL;
    assert(source_operation_root_region(&scratch, &wanted, NULL, 0, &witness) == SOURCE_OPERATION_ROOTS_COVERAGE);
    providers[0].kind = providers[1].kind = MODULE_ROOT_STANDARD;
    assert(source_operation_root_region(&scratch, &wanted, providers, 2, &witness) == SOURCE_OPERATION_ROOTS_COVERAGE);
    source_arena_destroy(&scratch); destroy_test();
}
