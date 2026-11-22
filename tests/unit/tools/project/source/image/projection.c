/** @file
 * @brief Check simultaneous material projection against independently stated final memberships.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "../../../../../../tools/project/source/image/projection.h"
/** Exact key lookup in test assertions, independent of production lookup. */
static const SourceCatalogImageEntry *row(const SourceCatalogImage *image, const char *path) {
    for (size_t i = 0; i < image->count; i++) { if (!strcmp(image->entries[i].path, path)) { return &image->entries[i]; } }
    return NULL;
}
/** Rename an existing selected child, replace a main's bytes and add a nested
 * main; every permutation yields the same exact final catalog and memberships. */
static void it_projects_simultaneous_changes_in_every_order_through_common_catalog(void) {
    setup_test(); ViewModel model; model_init(&model);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
    SourceCatalogImage original = {0}; SourceImageIssue shape;
    assert(source_catalog_image_prepare(&original, rows, VIEW_NODE_COUNT + 1, image_roots, 2, &shape) == SOURCE_IMAGE_OK);
    SourceImageChange edits[] = {
        {SOURCE_IMAGE_REMOVE, {.path = nodes[10].path}},
        {SOURCE_IMAGE_ADD, rows[10]}, {SOURCE_IMAGE_UPDATE, rows[8]},
        {SOURCE_IMAGE_ADD, {.path = "/virtual/packages/vendor/pkg/v1.0.0/src/new", .metadata = model.directory, .metadata_captured = true, .children_complete = true}},
        {SOURCE_IMAGE_ADD, rows[10]}};
    edits[1].entry.path = "/virtual/packages/vendor/pkg/v1.0.0/src/state+/renamed";
    edits[1].entry.metadata.st_ino = 1000;
    edits[2].entry.content = model.texts[21];
    edits[3].entry.metadata.st_ino = 1001;
    edits[4].entry.path = "/virtual/packages/vendor/pkg/v1.0.0/src/new/leaf";
    edits[4].entry.metadata.st_ino = 1002;
    for (unsigned encoded = 0; encoded < 3125; encoded++) {
        unsigned value = encoded; bool used[5] = {0}, unique = true; SourceImageChange ordered[5];
        for (size_t i = 0; i < 5; i++) { size_t index = value % 5; value /= 5; unique &= !used[index]; used[index] = true; ordered[i] = edits[index]; }
        if (!unique) { continue; }
        SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceCatalogImage final = {0}; SourceProjectionIssue issue;
        assert(source_image_project(&arena, &original, ordered, 5, &final, &issue) == SOURCE_PROJECTION_OK);
        assert(final.count == original.count + 2 && !row(&final, nodes[10].path));
        assert(row(&final, edits[1].entry.path) && row(&final, edits[4].entry.path));
        assert(row(&final, nodes[8].path)->content == model.texts[21]);
        assert(row(&original, nodes[8].path)->content == model.texts[8]);
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        fixture_io_begin(0);
        assert(module_catalog_build_view(&catalog, image_roots, 2, &final.reader) == MODULE_OK);
        const ModuleSource *source = NULL;
        assert(module_catalog_find(&catalog, "/vendor/pkg/v1.0.0/src/new/leaf", &source) == MODULE_OK);
        assert(module_catalog_find(&catalog, "/vendor/pkg/v1.0.0/src/state/renamed", &source) == MODULE_OK);
        assert(module_catalog_find(&catalog, "/vendor/pkg/v1.0.0/src/state/child", &source) == MODULE_MISSING);
        assert(fixture_io_end() == 0);
        module_catalog_destroy(&catalog); source_arena_destroy(&arena);
    }
    destroy_test();
}
/** Unknown membership/metadata, duplicate edits, absent/existing targets, roots
 * and orphan children refuse atomically, without repairing observations. */
static void it_refuses_unproved_or_conflicting_projection_without_publishing(void) {
    setup_test(); ViewModel model; model_init(&model);
    const SourceProjectionStatus expected[] = {SOURCE_PROJECTION_EXISTS, SOURCE_PROJECTION_MISSING,
        SOURCE_PROJECTION_UNOBSERVED, SOURCE_PROJECTION_UNOBSERVED, SOURCE_PROJECTION_DUPLICATE,
        SOURCE_PROJECTION_ROOT, SOURCE_PROJECTION_SHAPE, SOURCE_PROJECTION_UNOBSERVED,
        SOURCE_PROJECTION_INVALID, SOURCE_PROJECTION_UNOBSERVED, SOURCE_PROJECTION_UNOBSERVED,
        SOURCE_PROJECTION_UNOBSERVED, SOURCE_PROJECTION_INVALID, SOURCE_PROJECTION_SHAPE};
    for (size_t profile = 0; profile < sizeof(expected) / sizeof(*expected); profile++) {
        SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
        SourceImageChange edits[2] = {{SOURCE_IMAGE_UPDATE, rows[8]}, {SOURCE_IMAGE_UPDATE, rows[8]}};
        size_t count = 1;
        if (profile == 0) { edits[0].kind = SOURCE_IMAGE_ADD; }
        if (profile == 1 || profile == 2) { edits[0].entry.path = "/virtual/packages/vendor/pkg/v1.0.0/src/absent"; }
        if (profile == 2) { rows[5].children_complete = false; }
        if (profile == 3) { rows[8].metadata_captured = false; rows[8].content = NULL; }
        if (profile == 4) { count = 2; }
        if (profile == 5) { edits[0] = (SourceImageChange){SOURCE_IMAGE_REMOVE, rows[0]}; }
        if (profile == 6) { edits[0] = (SourceImageChange){SOURCE_IMAGE_REMOVE, rows[9]}; }
        if (profile == 7) { edits[0].entry.metadata_captured = false; edits[0].entry.content = NULL; }
        if (profile == 8) { edits[0].kind = (SourceImageChangeKind)99; }
        if (profile == 9) { edits[0] = (SourceImageChange){SOURCE_IMAGE_REMOVE, rows[9]}; rows[9].children_complete = false; }
        if (profile == 10) { edits[0] = (SourceImageChange){SOURCE_IMAGE_ADD, rows[8]}; edits[0].entry.path = "/virtual/dependencies/vendor/pkg/v1.0.0/new/leaf"; }
        if (profile == 11) { rows[5].children_complete = false; edits[0].kind = SOURCE_IMAGE_ADD; edits[0].entry.path = "/virtual/packages/vendor/pkg/v1.0.0/src/new/leaf"; }
        if (profile == 12) { edits[0].entry.path = NULL; }
        if (profile == 13) { edits[0].entry.path = "/virtual/packages/vendor/pkg/v1.0.0/src/new/"; edits[0].kind = SOURCE_IMAGE_ADD; }
        SourceCatalogImage original = {0}, output = {.count = 123}; SourceImageIssue shape; SourceProjectionIssue issue;
        assert(source_catalog_image_prepare(&original, rows, VIEW_NODE_COUNT + 1, image_roots, 2, &shape) == SOURCE_IMAGE_OK);
        SourceArena arena; source_arena_init(&arena, sources.allocator);
        assert(source_image_project(&arena, &original, edits, count, &output, &issue) == expected[profile]);
        assert(!output.valid && output.count == 123);
        source_arena_destroy(&arena);
    }
    destroy_test();
}
/** Single-vector allocation failure, empty domains, no-op copies and complete
 * subtree removal preserve original/output lifetime and independent membership. */
static void it_checks_projection_ownership_allocation_and_empty_interfaces(void) {
    setup_test(); ViewModel model; model_init(&model);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
    SourceCatalogImage original = {0}; SourceImageIssue shape; SourceProjectionIssue issue;
    assert(source_catalog_image_prepare(&original, rows, VIEW_NODE_COUNT + 1, image_roots, 2, &shape) == SOURCE_IMAGE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceCatalogImage output = {.count = 123};
    failure_at = allocation_count + 1;
    assert(source_image_project(&arena, &original, NULL, 0, &output, &issue) == SOURCE_PROJECTION_ALLOCATION);
    assert(output.count == 123 && !output.valid); failure_at = 0;
    assert(source_image_project(&arena, &original, NULL, 0, &output, &issue) == SOURCE_PROJECTION_OK);
    assert(output.entries != original.entries && output.count == original.count);
    for (size_t i = 0; i < original.count; i++) {
        assert(output.entries[i].path == original.entries[i].path && output.entries[i].content == original.entries[i].content);
    }
    SourceImageChange cuts[] = {{SOURCE_IMAGE_REMOVE, rows[9]}, {SOURCE_IMAGE_REMOVE, rows[10]}};
    SourceCatalogImage removed = {0};
    assert(source_image_project(&arena, &original, cuts, 2, &removed, &issue) == SOURCE_PROJECTION_OK);
    assert(removed.count == original.count - 2 && !row(&removed, nodes[9].path) && !row(&removed, nodes[10].path));
    assert(source_image_project(NULL, &original, NULL, 0, &output, &issue) == SOURCE_PROJECTION_INVALID);
    assert(source_image_project(&arena, NULL, NULL, 0, &output, &issue) == SOURCE_PROJECTION_INVALID);
    assert(source_image_project(&arena, &original, NULL, 0, NULL, &issue) == SOURCE_PROJECTION_INVALID);
    assert(source_image_project(&arena, &original, NULL, 0, &original, &issue) == SOURCE_PROJECTION_INVALID);
    assert(source_image_project(&arena, &original, NULL, 1, &output, &issue) == SOURCE_PROJECTION_INVALID);
    assert(source_image_project(&arena, &original, NULL, 0, &output, NULL) == SOURCE_PROJECTION_INVALID);
    SourceCatalogImage empty = {0}, result = {0};
    assert(source_catalog_image_prepare(&empty, NULL, 0, NULL, 0, &shape) == SOURCE_IMAGE_OK);
    size_t before = allocation_count;
    assert(source_image_project(&arena, &empty, NULL, 0, &result, &issue) == SOURCE_PROJECTION_OK);
    assert(result.valid && result.count == 0 && allocation_count == before);
    source_arena_destroy(&arena); destroy_test();
}
/** A regular original cannot have children. That exact absence proof permits
 * explicit file/directory replacement; shape checks still forbid orphan children. */
static void it_projects_file_directory_replacement_without_inventing_original_membership(void) {
    setup_test(); ViewModel model; model_init(&model);
    ModuleRoot root = {"/v", NULL, 0, MODULE_ROOT_STANDARD};
    for (unsigned reverse = 0; reverse < 2; reverse++) {
        SourceCatalogImageEntry rows[] = {
            {.path = "/v", .metadata = model.directory, .metadata_captured = true, .children_complete = true},
            {.path = "/v/data", .metadata = reverse ? model.directory : model.regular,
                .metadata_captured = true, .children_complete = reverse != 0, .content = reverse ? NULL : model.texts[8]},
            {.path = "/v/data/child", .metadata = model.regular, .metadata_captured = true, .content = model.texts[8]}};
        SourceCatalogImage original = {0}, final = {0}; SourceImageIssue shape; SourceProjectionIssue issue;
        assert(source_catalog_image_prepare(&original, rows, reverse ? 3 : 2, &root, 1, &shape) == SOURCE_IMAGE_OK);
        SourceImageChange edits[] = {
            {SOURCE_IMAGE_UPDATE, {.path = "/v/data", .metadata = reverse ? model.regular : model.directory,
                .metadata_captured = true, .children_complete = reverse == 0, .content = reverse ? model.texts[8] : NULL}},
            {reverse ? SOURCE_IMAGE_REMOVE : SOURCE_IMAGE_ADD, rows[2]}};
        SourceArena arena; source_arena_init(&arena, sources.allocator);
        assert(source_image_project(&arena, &original, edits, 2, &final, &issue) == SOURCE_PROJECTION_OK);
        assert(final.count == (reverse ? 2u : 3u));
        assert((row(&final, "/v/data/child") != NULL) == (reverse == 0));
        assert(source_image_project(&arena, &original, edits + 1, 1, &final, &issue) ==
            (reverse ? SOURCE_PROJECTION_OK : SOURCE_PROJECTION_SHAPE));
        if (!reverse) {
            SourceImageChange missing = {SOURCE_IMAGE_REMOVE, {.path = "/v/data/missing"}};
            assert(source_image_project(&arena, &original, &missing, 1, &final, &issue) == SOURCE_PROJECTION_MISSING);
        }
        source_arena_destroy(&arena);
    }
    destroy_test();
}
/** Export cases to the owning leaf; helpers remain private to this translation unit. */
static void test_image_projection(void) {
    it_projects_file_directory_replacement_without_inventing_original_membership();
    it_checks_projection_ownership_allocation_and_empty_interfaces();
    it_projects_simultaneous_changes_in_every_order_through_common_catalog();
    it_refuses_unproved_or_conflicting_projection_without_publishing();
}
