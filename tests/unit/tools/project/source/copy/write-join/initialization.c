/** @file
 * @brief Join genuine test-only copies with required empty source areas in a newly initialized version.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** A copied test is not required to bring tests-of-tests. Its new version still
 * needs an empty src area, so this exercises actual directory initialization
 * rather than silently treating missing areas as an empty package. */
static void it_joins_test_only_copies_with_required_new_version_areas(void) {
    prepare(); source_tree_selection_init(&tree, sources.allocator);
    const ModuleSource *test = NULL;
    assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/tests/main/check", &test) == MODULE_OK);
    assert(source_tree_selection_read(&tree, &capture, source_path(test->text)) == SOURCE_TREE_OK);
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    const char *version_path = source_arena_join(&arena, capture.roots[0].path, "/", "packages/target/new/v3.0.0"); assert(version_path);
    const char *path = source_arena_join(&arena, version_path, "/", "tests/copied"); assert(path);
    SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, &capture); const SourceCloneMapping *clone = NULL;
    assert(source_clone_map_add(&mains, test, path, "/target/new/v3.0.0/tests/copied", &clone) == SOURCE_CLONE_OK);
    assert(source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    SourceCopyContextOmissions contexts; source_copy_context_omissions_init(&contexts, sources.allocator, &copy, &mains); SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&files, &copy, &mains, &contexts, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_context_omissions_seal(&contexts) == SOURCE_COPY_CONTEXT_OK && source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    const SourceCatalogImageEntry *mode = NULL;
    for (size_t i = 0; i < observed.image.count; i++) {
        const SourceCatalogImageEntry *row = &observed.image.entries[i];
        if (!strcmp(row->path, source_path(test->version->configuration))) { mode = row; }
    }
    assert(mode); SourceVersionPlacement version = {"/target/new/v3.0.0", version_path};
    SourceVersionInitialization initialization; SourceVersionInitializationIssue issue;
    assert(source_version_initialize(&arena, &sources, &observed, capture.roots, capture.root_count,
        &version, &files, mode, NULL, 0, &initialization, &issue) == SOURCE_VERSION_INITIALIZATION_OK && initialization.count == 2);
    const SourceVersionInitialization *initializations[] = {&initialization};
    SourceOperationWritesInputs inputs = {.copy = &copy, .observed = &observed, .mains = &mains, .files = &files,
        .initializations = initializations, .initialization_count = 1};
    SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
    assert(source_operation_writes_build(&writes, &inputs) == SOURCE_OPERATION_WRITES_OK && writes.valid && writes.count == 2 && !writes.prepared);
    check_source_write_join_failures(&inputs);
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&arena, &observed.image, &files, writes.prepared, writes.changes,
        writes.count, capture.roots, capture.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    FILE *request = fopen("generated/test-only-initialization.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/test-only-initialization.json", &final.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&arena, &analysis, &observed, &mains, &files, NULL, &contexts, &final, &client, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    check_complete_retained_roots(&inputs, &contexts, &client, SOURCE_OPERATION_SCOPE_INITIALIZED);
    const SourceVersionInitialization *duplicates[] = {&initialization, &initialization}; inputs.initializations = duplicates; inputs.initialization_count = 2;
    SourceOperationWrites repeated; source_operation_writes_init(&repeated, sources.allocator);
    assert(source_operation_writes_build(&repeated, &inputs) == SOURCE_OPERATION_WRITES_DUPLICATE && !repeated.valid);
    source_operation_writes_destroy(&repeated);
    SourceVersionInitialization incomplete = initialization; incomplete.count = 1;
    const SourceVersionInitialization *missing[] = {&incomplete}; inputs.initializations = missing; inputs.initialization_count = 1;
    source_operation_writes_init(&repeated, sources.allocator);
    assert(source_operation_writes_build(&repeated, &inputs) == SOURCE_OPERATION_WRITES_SCOPE && !repeated.valid);
    const char *missing_source = source_arena_join(&arena, version_path, "/", "src"); assert(missing_source);
    assert(repeated.failed_path && !strcmp(repeated.failed_path, missing_source));
    source_operation_writes_destroy(&repeated); source_compiler_client_destroy(&client); source_image_capture_destroy(&final);
    source_operation_writes_destroy(&writes); source_file_map_destroy(&files); source_copy_context_omissions_destroy(&contexts);
    source_clone_map_destroy(&mains); source_observed_image_destroy(&observed); source_arena_destroy(&arena); finish_copy();
}
