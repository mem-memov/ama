/** @file
 * @brief Exhaust deepest test-association order, component, ownership and refusal partitions.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/tests.h"
#include "../../../../../../tools/project/acquisition/closure/tests/association.h"
/** Both orders use the deepest containing directory, never a similarly spelled
 * sibling or the module whose name equals the directory itself. */
static void it_maps_associated_tests_by_deepest_directory_independently_of_order(void) {
    static const char *const inputs[] = {
        "/a/p/v1.0.0/tests/tree/check", "/a/p/v1.0.0/tests/tree/branch/check",
        "/a/p/v1.0.0/tests/tree/branch/deep/check", "/a/p/v1.0.0/tests/tree/branchish/check",
        "/a/p/v1.0.0/tests/tree", "/a/p/v1.0.0/tests/treeish/check",
        "/a/p/v1.0.0/tests/tree/branch"};
    static const char *const expected[] = {
        "/b/q/v2.0.0/tests/renamed/check", "/b/q/v2.0.0/tests/leaf/check",
        "/b/q/v2.0.0/tests/leaf/deep/check", "/b/q/v2.0.0/tests/renamed/branchish/check",
        NULL, NULL, "/b/q/v2.0.0/tests/renamed/branch"};
    for (unsigned order = 0; order < 2; order++) {
        SourceTestDirectoryMap pairs[3] = {
            {"/a/p/v1.0.0/tests/tree", "/b/q/v2.0.0/tests/renamed"},
            {"/a/p/v1.0.0/tests/tree/branch", "/b/q/v2.0.0/tests/leaf"},
            {"/a/p/v1.0.0/tests/tree", "/b/q/v2.0.0/tests/renamed"}};
        if (order) { SourceTestDirectoryMap swap = pairs[0]; pairs[0] = pairs[1]; pairs[1] = swap; }
        prepare();
        for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
            const char *output = "sentinel";
            SourceTestMapStatus status = source_associated_test_identity(&capture.arena, pairs, 3, inputs[i], &output);
            assert(status == (expected[i] ? SOURCE_TEST_MAP_OK : SOURCE_TEST_MAP_OUTSIDE));
            assert(!strcmp(output, expected[i] ? expected[i] : "sentinel"));
        }
        finish();
    }
}
/** Validate every declaration even if unrelated; do not allocate on invalid,
 * contradictory or unmatched inputs. Failed owned output is never published. */
static void it_refuses_invalid_conflicting_and_failed_test_directory_projections(void) {
    prepare();
    SourceTestDirectoryMap pairs[] = {{"/a/p/v1.0.0/tests/tree", "/b/q/v2.0.0/tests/renamed"},
        {"/a/p/v1.0.0/tests/tree", "/b/q/v2.0.0/tests/other"}};
    const char *identity = "/a/p/v1.0.0/tests/tree/check", *output = "sentinel";
    size_t before = allocation_count;
    assert(source_associated_test_identity(&capture.arena, pairs, 2, identity, &output) == SOURCE_TEST_MAP_CONFLICT);
    pairs[1].original = "/a/p/v1.0.0/src/unrelated";
    assert(source_associated_test_identity(&capture.arena, pairs, 2, identity, &output) == SOURCE_TEST_MAP_INVALID);
    pairs[1].original = NULL;
    assert(source_associated_test_identity(&capture.arena, pairs, 2, identity, &output) == SOURCE_TEST_MAP_INVALID);
    assert(source_associated_test_identity(&capture.arena, NULL, 1, identity, &output) == SOURCE_TEST_MAP_INVALID);
    assert(source_associated_test_identity(NULL, pairs, 1, identity, &output) == SOURCE_TEST_MAP_INVALID);
    assert(source_associated_test_identity(&capture.arena, pairs, 1, identity, NULL) == SOURCE_TEST_MAP_INVALID);
    assert(source_associated_test_identity(&capture.arena, pairs, 1, "//plain", &output) == SOURCE_TEST_MAP_INVALID);
    assert(source_associated_test_identity(&capture.arena, NULL, 0, identity, &output) == SOURCE_TEST_MAP_OUTSIDE);
    assert(allocation_count == before && !strcmp(output, "sentinel"));
    failure_at = allocation_count + 1;
    assert(source_associated_test_identity(&capture.arena, pairs, 1, identity, &output) == SOURCE_TEST_MAP_MEMORY);
    assert(!strcmp(output, "sentinel")); failure_at = 0;
    char destination[] = "/b/q/v2.0.0/tests/renamed"; pairs[0].destination = destination;
    assert(source_associated_test_identity(&capture.arena, pairs, 1, identity, &output) == SOURCE_TEST_MAP_OK);
    destination[1] = 'z'; assert(!strcmp(output, "/b/q/v2.0.0/tests/renamed/check"));
    finish();
}

/** Actual catalog originals and explicit clone rows produce associations through
 * the same logical namespace operation as delivery, without physical version mocks. */
static void it_composes_catalog_clone_rows_with_shared_logical_test_associations(void) {
    prepare(); source_clone_map_init(&map, sources.allocator, &capture);
    const SourceCloneMapping *row = NULL;
    assert(source_clone_map_add(&map, original("/vendor/pkg/v1.0.0/src/parent/child"),
        "/new/version/src/moved", "/target/pkg/v2.0.0/src/moved", &row) == SOURCE_CLONE_OK);
    assert(source_clone_map_add(&map, original("/vendor/pkg/v1.0.0/src/parentish"),
        "/new/version/src/other", "/target/pkg/v2.0.0/src/other", &row) == SOURCE_CLONE_OK);
    SourceTestDirectoryMap pairs[2]; size_t count = 0;
    for (row = map.first; row; row = row->next) {
        assert(count < 2);
        assert(acquisition_test_namespace(&capture.arena, row->original->logical_path,
            &pairs[count].original) == SOURCE_OK);
        assert(acquisition_test_namespace(&capture.arena, row->identity,
            &pairs[count].destination) == SOURCE_OK);
        count++;
    }
    assert(count == 2);
    const char *output = NULL;
    assert(source_associated_test_identity(&capture.arena, pairs, count,
        "/vendor/pkg/v1.0.0/tests/parent/child/check", &output) == SOURCE_TEST_MAP_OK);
    assert(!strcmp(output, "/target/pkg/v2.0.0/tests/moved/check"));
    assert(source_associated_test_identity(&capture.arena, pairs, count,
        "/vendor/pkg/v1.0.0/tests/parentish/support/check", &output) == SOURCE_TEST_MAP_OK);
    assert(!strcmp(output, "/target/pkg/v2.0.0/tests/other/support/check"));
    source_clone_map_destroy(&map); finish();
}
