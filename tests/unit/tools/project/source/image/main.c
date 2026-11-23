/** @file
 * @brief Verify prepared-image completeness, original alias classes and exact unavailable/absent distinctions.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "projection.c"
#include "domains.c"
/** An independent callback model and both image row orders feed the same common
 * discovery/companion pipeline with identical selected source bytes and aliases. */
static void it_matches_independent_complete_catalog_views_in_both_image_orders_without_disk_io(void) {
    setup_test(); ViewModel model; model_init(&model); ModuleCatalogReader oracle = reader_for(&model);
    ModuleCatalog expected; module_catalog_init(&expected, sources.allocator);
    assert(prepare_catalog(&expected, &oracle) == MODULE_OK);
    for (unsigned order = 0; order < 2; order++) {
        SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
        if (order) {
            for (size_t i = 0; i < (VIEW_NODE_COUNT + 1) / 2; i++) {
                SourceCatalogImageEntry swap = rows[i]; rows[i] = rows[VIEW_NODE_COUNT - i]; rows[VIEW_NODE_COUNT - i] = swap;
            }
        }
        SourceCatalogImage image = {0}; SourceImageIssue issue = {0}; size_t before = allocation_count;
        assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 1, image_roots, 2, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
        assert(image.valid && image.reader.tree.context == &image && allocation_count == before);
        ModuleCatalog actual; module_catalog_init(&actual, sources.allocator);
        assert(prepare_catalog(&actual, &image.reader) == MODULE_OK);
        same_catalog(&expected, &actual);
        const ModuleSource *state = NULL;
        assert(module_catalog_find(&actual, "/vendor/pkg/v1.0.0/src/state", &state) == MODULE_OK);
        assert(state->configuration && state->native_loaded && state->native_c && state->native_header && state->native_abi);
        assert(actual.issue.status == MODULE_OK);
        module_catalog_destroy(&actual);
    }
    module_catalog_destroy(&expected); destroy_test();
}
/** Known unselected names need no metadata/bytes; selected unknown metadata,
 * unread contents and incomplete directories cannot masquerade as absence. */
static void it_refuses_unavailable_selected_image_inputs_and_preserves_common_alias_and_anchor_checks(void) {
    const ModuleStatus expected[] = {MODULE_IO_ERROR, MODULE_IO_ERROR, MODULE_IO_ERROR,
        MODULE_IO_ERROR, MODULE_FILE_ALIAS, MODULE_FILE_ALIAS, MODULE_ORPHAN_DIRECTORY, MODULE_MISSING,
        MODULE_UNSUPPORTED_PATH};
    for (size_t profile = 0; profile < sizeof(expected) / sizeof(expected[0]); profile++) {
        setup_test(); ViewModel model; model_init(&model);
        SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows); size_t count = VIEW_NODE_COUNT + 1;
        if (profile == 0) { rows[8].metadata_captured = false; rows[8].content = NULL; }
        if (profile == 1) { rows[8].content = NULL; }
        if (profile == 2) { rows[5].children_complete = false; }
        if (profile == 3) { rows[7].metadata_captured = false; rows[7].content = NULL; }
        if (profile == 4) { rows[10].metadata.st_ino = rows[8].metadata.st_ino; }
        if (profile == 5) { rows[15].metadata.st_ino = rows[9].metadata.st_ino; }
        if (profile == 6 || profile == 7) { rows[profile == 6 ? 8 : 7] = rows[--count]; }
        if (profile == 8) { rows[8].metadata.st_mode = 0; rows[8].content = NULL; }
        SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
        assert(source_catalog_image_prepare(&image, rows, count, image_roots, 2, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        assert(prepare_catalog(&catalog, &image.reader) == expected[profile]);
        module_catalog_destroy(&catalog); destroy_test();
    }
}
/** Direct exact probes distinguish unknown metadata, complete absence and
 * incomplete listings. Outputs remain untouched on every unsuccessful callback. */
static void it_distinguishes_image_absence_from_unavailable_observations_without_fallback(void) {
    setup_test(); ViewModel model; model_init(&model);
    for (unsigned complete = 0; complete < 2; complete++) {
        SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 2]; image_rows(&model, rows);
        rows[VIEW_NODE_COUNT + 1] = (SourceCatalogImageEntry){
            .path = "/virtual/packages/vendor/pkg/v1.0.0/src/empty", .metadata = model.directory,
            .metadata_captured = true, .children_complete = true};
        rows[VIEW_NODE_COUNT + 1].metadata.st_dev = 1; rows[VIEW_NODE_COUNT + 1].metadata.st_ino = 1000;
        rows[5].children_complete = complete != 0;
        SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
        assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 2, image_roots, 2, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        struct stat output = model.regular; ino_t before = output.st_ino;
        fixture_io_begin(0);
        assert(image.reader.tree.child(&image, &catalog, nodes[5].path, "absent",
            "/virtual/packages/vendor/pkg/v1.0.0/src/absent", &output) == (complete ? MODULE_MISSING : MODULE_IO_ERROR));
        assert(output.st_ino == before);
        assert(image.reader.tree.root(&image, &catalog, "/outside", &output) == MODULE_INVALID_PATH);
        assert(image.reader.tree.child(&image, &catalog, nodes[5].path, "different", nodes[8].path, &output) == MODULE_INVALID_PATH);
        assert(image.reader.tree.root(NULL, &catalog, nodes[5].path, &output) == MODULE_INVALID_PATH);
        assert(image.reader.tree.root(&image, &catalog, nodes[8].path, &output) == MODULE_UNSUPPORTED_PATH);
        assert(image.reader.tree.child(&image, &catalog, NULL, "x", "/x", &output) == MODULE_INVALID_PATH);
        assert(image.reader.tree.child(&image, &catalog, nodes[5].path, "a/b", "/a/b", &output) == MODULE_INVALID_PATH);
        assert(image.reader.tree.child(&image, &catalog, nodes[19].path, "v1.0.0",
            "/virtual/dependencies/vendor/pkg/v1.0.0", &output) == MODULE_IO_ERROR);
        const char *sentinel_names[] = {"sentinel"}; const char **names = sentinel_names; size_t count = 99;
        assert(image.reader.tree.entries(&image, &catalog, rows[VIEW_NODE_COUNT + 1].path, &names, &count) == MODULE_OK);
        assert(!names && !count);
        const SourceText *text = model.texts[8];
        assert(image.reader.read(&image, nodes[12].path, &text) == SOURCE_IO_ERROR && text == model.texts[8]);
        assert(image.reader.read(&image, "/unknown", &text) == SOURCE_IO_ERROR && text == model.texts[8]);
        assert(image.reader.read(NULL, nodes[8].path, &text) == SOURCE_INVALID_ARGUMENT);
        assert(image.reader.read(&image, NULL, &text) == SOURCE_INVALID_ARGUMENT);
        assert(image.reader.read(&image, nodes[8].path, NULL) == SOURCE_INVALID_ARGUMENT);
        assert(fixture_io_end() == 0);
        module_catalog_destroy(&catalog);
    }
    destroy_test();
}
/** Image preparation is allocation-free; every common-catalog/image-name-array
 * allocation can fail independently and retire to the original snapshot baseline. */
static void it_retires_every_prepared_image_catalog_allocation_failure(void) {
    setup_test(); ViewModel model; model_init(&model);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
    SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
    assert(source_catalog_image_prepare(&image, rows, VIEW_NODE_COUNT + 1, image_roots, 2, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
    size_t allocations = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        ModuleStatus status = prepare_catalog(&catalog, &image.reader);
        if (!failure) { assert(status == MODULE_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == MODULE_OUT_OF_MEMORY && catalog.issue.status == MODULE_OUT_OF_MEMORY); }
        failure_at = 0; module_catalog_destroy(&catalog); assert(live_allocations == baseline);
    }
    destroy_test();
}
/** Reject inconsistent row/parent/root contracts before publication, including
 * NULL keys before lookup, while empty prepared domains remain valid. */
static void it_rejects_malformed_prepared_images_without_overwriting_the_adapter(void) {
    setup_test(); ViewModel model; model_init(&model);
    for (unsigned profile = 0; profile < 11; profile++) {
        SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 2]; image_rows(&model, rows);
        size_t count = VIEW_NODE_COUNT + 1;
        SourceImageStatus expected = SOURCE_IMAGE_INVALID;
        if (profile == 0) { rows[0].path = NULL; }
        if (profile == 1) { rows[1] = rows[0]; expected = SOURCE_IMAGE_DUPLICATE; }
        if (profile == 2) { rows[count++] = (SourceCatalogImageEntry){.path = "/outside"}; expected = SOURCE_IMAGE_OUTSIDE; }
        if (profile == 3) { rows[2] = rows[--count]; expected = SOURCE_IMAGE_PARENT; }
        if (profile == 4) { rows[8].metadata_captured = false; }
        if (profile == 5) { rows[5].content = model.texts[8]; }
        if (profile == 6) { rows[8].children_complete = true; }
        if (profile == 7) { rows[8].path = "relative"; }
        if (profile == 8) { rows[8].path = "/virtual/trailing/"; }
        if (profile == 9) { rows[0].metadata = model.regular; rows[0].children_complete = false; expected = SOURCE_IMAGE_PARENT; }
        if (profile == 10) { rows[9].metadata = model.regular; rows[9].children_complete = false; expected = SOURCE_IMAGE_PARENT; }
        SourceCatalogImage image = {.count = 123}; SourceImageIssue issue = {0}; size_t before = allocation_count;
        assert(source_catalog_image_prepare(&image, rows, count, image_roots, 2, (SourceImageBounds){0}, &issue) == expected);
        assert(image.count == 123 && !image.valid && allocation_count == before);
    }
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
    for (unsigned profile = 0; profile < 5; profile++) {
        ModuleRoot roots[2] = {image_roots[0], image_roots[1]}; const char *excluded[] = {NULL};
        if (profile == 0) { roots[0].path = NULL; }
        if (profile == 1) { roots[0].path = "/missing"; }
        if (profile == 2) { roots[0].excluded_count = 1; }
        if (profile == 3) { roots[0].excluded_count = 1; roots[0].excluded = excluded; }
        if (profile == 4) { roots[0].kind = (ModuleRootKind)99; }
        SourceCatalogImage unchanged = {.count = 123}; SourceImageIssue root_issue = {0};
        assert(source_catalog_image_prepare(&unchanged, rows, VIEW_NODE_COUNT + 1, roots, 2, (SourceImageBounds){0}, &root_issue) ==
            (profile == 1 ? SOURCE_IMAGE_PARENT : SOURCE_IMAGE_INVALID));
        assert(unchanged.count == 123 && !unchanged.valid && root_issue.path == roots[0].path);
    }
    SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
    assert(source_catalog_image_prepare(NULL, NULL, 0, NULL, 0, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_INVALID);
    assert(source_catalog_image_prepare(&image, NULL, 1, NULL, 0, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_INVALID);
    assert(source_catalog_image_prepare(&image, NULL, 0, NULL, 1, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_INVALID);
    assert(source_catalog_image_prepare(&image, NULL, 0, NULL, 0, (SourceImageBounds){0}, NULL) == SOURCE_IMAGE_INVALID);
    assert(source_catalog_image_prepare(&image, NULL, 0, NULL, 0, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK && image.valid);
    ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
    assert(module_catalog_build_view(&catalog, image.roots, image.root_count, &image.reader) == MODULE_OK);
    module_catalog_destroy(&catalog); destroy_test();
}
/** The shared scanner appends separators to exact raw root prefixes. Preserve
 * those generated physical keys instead of collapsing POSIX root spellings. */
static void it_preserves_single_and_repeated_slash_root_prefixes_through_common_discovery(void) {
    setup_test(); ViewModel model; model_init(&model);
    assert(!source_image_below(NULL, "/") && !source_image_below("/x", NULL));
    assert(!source_image_below("/x", "") && !source_image_below("relative", "/"));
    assert(source_image_below("/root/a", "/root") && !source_image_below("/rootish/a", "/root"));
    const char *const roots[] = {"/", "//", "///"};
    const char *const mains[] = {"//integer", "///integer", "////integer"};
    for (size_t i = 0; i < 3; i++) {
        SourceCatalogImageEntry rows[] = {
            {.path = roots[i], .metadata = model.directory, .metadata_captured = true, .children_complete = true},
            {.path = mains[i], .metadata = model.regular, .metadata_captured = true, .content = model.texts[21]}};
        rows[0].metadata.st_dev = rows[1].metadata.st_dev = 1;
        rows[0].metadata.st_ino = 1; rows[1].metadata.st_ino = 2;
        ModuleRoot root = {roots[i], NULL, 0, MODULE_ROOT_STANDARD};
        SourceCatalogImage image = {0}; SourceImageIssue issue = {0};
        assert(source_catalog_image_prepare(&image, rows, 2, &root, 1, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        fixture_io_begin(0);
        assert(module_catalog_build_view(&catalog, &root, 1, &image.reader) == MODULE_OK);
        const ModuleSource *source = NULL;
        assert(module_catalog_find(&catalog, "//integer", &source) == MODULE_OK);
        assert(!strcmp(source_path(source->text), mains[i]) && fixture_io_end() == 0);
        module_catalog_destroy(&catalog);
    }
    destroy_test();
}
/** Run independent-view equality, complete/unavailable distinctions and all fault domains. */
void run(void) {
    it_separates_module_roots_from_observation_envelopes_without_hidden_io();
    it_refuses_malformed_or_unobserved_physical_envelopes();
    test_image_projection();
    it_preserves_single_and_repeated_slash_root_prefixes_through_common_discovery();
    it_matches_independent_complete_catalog_views_in_both_image_orders_without_disk_io();
    it_refuses_unavailable_selected_image_inputs_and_preserves_common_alias_and_anchor_checks();
    it_distinguishes_image_absence_from_unavailable_observations_without_fallback();
    it_retires_every_prepared_image_catalog_allocation_failure();
    it_rejects_malformed_prepared_images_without_overwriting_the_adapter();
}
/** Every borrowed image expires after its catalog, then original immutable bytes. */
int main(void) { run(); return 0; }
