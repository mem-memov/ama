/** @file
 * @brief Exercise semantic reselection and physical-root edits inside a wider observation envelope.
 * @see [T275 contract](../../../../../../docs/implementation/source-operations.md#t275-entry-review-and-execution)
 */
/** A stable parent observes invalid unrelated bytes without indexing a module;
 * root-only selection changes keep every physical entry and the same envelope. */
static void it_separates_module_roots_from_observation_envelopes_without_hidden_io(void) {
    setup_test(); ViewModel model; model_init(&model);
    const SourceText *valid = model.texts[21], *invalid = NULL;
    assert(source_manager_add(&sources, "/observe/unrelated", (const unsigned char *)"invalid syntax", 14, &invalid) == SOURCE_OK);
    SourceCatalogImageEntry rows[] = {
        {.path = "/observe", .metadata = model.directory, .metadata_captured = true, .children_complete = true},
        {.path = "/observe/selected", .metadata = model.directory, .metadata_captured = true, .children_complete = true},
        {.path = "/observe/selected/plain", .metadata = model.regular, .metadata_captured = true, .content = valid},
        {.path = "/observe/unrelated", .metadata = model.regular, .metadata_captured = true, .content = invalid}};
    for (size_t i = 0; i < 4; i++) { rows[i].metadata.st_dev = 1; rows[i].metadata.st_ino = (ino_t)(i + 1); }
    ModuleRoot selected = {"/observe/selected", NULL, 0, MODULE_ROOT_STANDARD};
    const char *paths[] = {"/observe"}; SourceImageBounds bounds = {paths, 1};
    SourceCatalogImage original, final; SourceImageIssue issue;
    assert(source_catalog_image_prepare(&original, rows, 4, &selected, 1, bounds, &issue) == SOURCE_IMAGE_OK);
    assert(source_image_bound_count(&original) == 1 && !strcmp(source_image_bound_path(&original, 0), paths[0]));
    ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator); fixture_io_begin(0);
    assert(module_catalog_build_view(&catalog, original.roots, original.root_count, &original.reader) == MODULE_OK);
    assert(catalog.first && !catalog.first->next && !strcmp(catalog.first->logical_path, "//plain"));
    assert(fixture_io_end() == 0); module_catalog_destroy(&catalog);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourceProjectionIssue projection;
    assert(source_image_project(&arena, &original, NULL, 0, NULL, 0, &final, &projection) == SOURCE_PROJECTION_OK);
    assert(final.count == 4 && !final.root_count && source_image_same_bounds(&original, &final));
    module_catalog_init(&catalog, sources.allocator);
    assert(module_catalog_build_view(&catalog, final.roots, final.root_count, &final.reader) == MODULE_OK && !catalog.first);
    module_catalog_destroy(&catalog);
    SourceImageChange removal[] = {{SOURCE_IMAGE_REMOVE, rows[1]}, {SOURCE_IMAGE_REMOVE, rows[2]}};
    assert(source_image_project(&arena, &original, removal, 2, NULL, 0, &final, &projection) == SOURCE_PROJECTION_OK && final.count == 2);
    assert(source_image_project(&arena, &original, removal, 1, NULL, 0, &final, &projection) == SOURCE_PROJECTION_SHAPE);
    removal[0].entry = rows[0];
    assert(source_image_project(&arena, &original, removal, 1, NULL, 0, &final, &projection) == SOURCE_PROJECTION_ROOT);
    SourceImageChange added[] = {{SOURCE_IMAGE_ADD, rows[1]}, {SOURCE_IMAGE_ADD, rows[2]}};
    added[0].entry.path = "/observe/new"; added[1].entry.path = "/observe/new/plain";
    added[0].entry.metadata.st_ino = 10; added[1].entry.metadata.st_ino = 11;
    ModuleRoot destination = {"/observe/new", NULL, 0, MODULE_ROOT_STANDARD};
    assert(source_image_project(&arena, &original, added, 2, &destination, 1, &final, &projection) == SOURCE_PROJECTION_OK);
    assert(final.count == 6 && source_image_same_bounds(&original, &final));
    module_catalog_init(&catalog, sources.allocator);
    assert(module_catalog_build_view(&catalog, final.roots, final.root_count, &final.reader) == MODULE_OK);
    assert(catalog.first && !catalog.first->next && !strcmp(source_path(catalog.first->text), "/observe/new/plain"));
    module_catalog_destroy(&catalog); source_arena_destroy(&arena); destroy_test();
}
/** Reject missing/foreign boundary authority before replacing the output adapter. */
static void it_refuses_malformed_or_unobserved_physical_envelopes(void) {
    setup_test(); ViewModel model; model_init(&model);
    SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]; image_rows(&model, rows);
    const char *paths[] = {NULL, "relative", "/missing", "/virtual/packages"};
    const SourceImageStatus expected[] = {SOURCE_IMAGE_INVALID, SOURCE_IMAGE_INVALID, SOURCE_IMAGE_PARENT, SOURCE_IMAGE_OUTSIDE};
    for (size_t i = 0; i < 4; i++) {
        SourceCatalogImage output = {.count = 99}; SourceImageIssue issue;
        SourceImageBounds bounds = {&paths[i], 1};
        assert(source_catalog_image_prepare(&output, rows, VIEW_NODE_COUNT + 1, image_roots, 2, bounds, &issue) == expected[i]);
        assert(output.count == 99 && !output.valid);
    }
    SourceCatalogImage output = {.count = 99}; SourceImageIssue issue;
    assert(source_catalog_image_prepare(&output, rows, VIEW_NODE_COUNT + 1, image_roots, 2,
        (SourceImageBounds){NULL, 1}, &issue) == SOURCE_IMAGE_INVALID && output.count == 99);
    destroy_test();
}
