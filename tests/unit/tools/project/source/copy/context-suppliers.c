/** @file
 * @brief Prove unchanged formerly ignored structural providers without manufacturing original selections.
 * @see [T276 resumed review](../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Build both real physical suppliers before the original common catalog ignores
 * the external one. Its empty package configuration imports no hidden permissions. */
static void prepare_supplier_context(void) {
    setup_test(); fixtures();
    const char *paths[] = {"generated/external", "generated/external/dependencies", "generated/external/dependencies/vendor",
        "generated/external/dependencies/vendor/pkg", "generated/external/dependencies/vendor/pkg/v1.0.0",
        "generated/external/dependencies/vendor/pkg/v1.0.0/src", "generated/external/dependencies/vendor/pkg/v1.0.0/tests",
        "generated/external/dependencies/vendor/pkg/v1.0.0/src/unobserved"};
    for (size_t i = 0; i < sizeof(paths)/sizeof(*paths); i++) { directory(paths[i]); }
    file("generated/external/dependencies/vendor/pkg/v1.0.0/package.ini", "");
    file("generated/external/dependencies/vendor/pkg/v1.0.0/src/main", "@!\n");
    file("generated/external/dependencies/vendor/pkg/v1.0.0/src/other", "@!\n");
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}, {"generated/external", NULL, 0, MODULE_ROOT_PROJECT}};
    capture_fixtures_roots(roots, 3); select_child_context();
}
/** Retire only the explicitly constructed external supplier fixture. */
static void destroy_supplier_context_files(void) {
    assert(unlink("generated/external/dependencies/vendor/pkg/v1.0.0/src/main") == 0);
    assert(unlink("generated/external/dependencies/vendor/pkg/v1.0.0/src/other") == 0);
    assert(unlink("generated/external/dependencies/vendor/pkg/v1.0.0/package.ini") == 0);
    const char *directories[] = {"generated/external/dependencies/vendor/pkg/v1.0.0/src/unobserved",
        "generated/external/dependencies/vendor/pkg/v1.0.0/tests",
        "generated/external/dependencies/vendor/pkg/v1.0.0/src", "generated/external/dependencies/vendor/pkg/v1.0.0",
        "generated/external/dependencies/vendor/pkg", "generated/external/dependencies/vendor",
        "generated/external/dependencies", "generated/external"};
    for (size_t i = 0; i < sizeof(directories)/sizeof(*directories); i++) { assert(rmdir(directories[i]) == 0); }
}
/** This certifies source/provenance facts of explicit original/final views only;
 * final root-setting authorization and full command intent remain T276 gates. */
static void it_proves_unchanged_previously_ignored_parents_as_structural_providers(void) {
    prepare_supplier_context(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    const char *external = source_arena_join(&arena, capture.roots[2].path, "/", "dependencies/vendor/pkg/v1.0.0"); assert(external);
    SourceObservationRequest requests[7];
    const char *suffixes[] = {"", "/package.ini", "/src", "/tests", "/src/main", "/src/other", "/src/unobserved"};
    for (size_t i = 0; i < 7; i++) {
        const char *path = source_arena_join(&arena, external, "", suffixes[i]); assert(path);
        requests[i] = (SourceObservationRequest){path, i == 1 || i == 4 || i == 5 ? SOURCE_OBSERVE_CONTENT : SOURCE_OBSERVE_CHILDREN, false};
    }
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, requests, 7) == SOURCE_OBSERVATION_OK);
    SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, &capture);
    const char *names[] = {child_identity, "/vendor/pkg/v1.0.0/tests/parent/child/check"};
    const char *destinations[] = {"/src/other+/copied", "/tests/other/copied/check"};
    const char *identities[] = {"/vendor/pkg/v1.0.0/src/other/copied", "/vendor/pkg/v1.0.0/tests/other/copied/check"};
    for (size_t i = 0; i < 2; i++) {
        const ModuleSource *source = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, names[i], &source) == MODULE_OK);
        const char *path = source_arena_join(&arena, external, "", destinations[i]); assert(path);
        assert(source_clone_map_add(&mains, source, path, identities[i], &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
    SourceCopyContextOmissions omissions; source_copy_context_omissions_init(&omissions, sources.allocator, &copy, &mains);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&files, &copy, &mains, &omissions, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_context_omissions_seal(&omissions) == SOURCE_COPY_CONTEXT_OK && omissions.first);
    SourceVersionPlacement version = {"/vendor/pkg/v1.0.0", external}; SourceCopyTestsIssue tests;
    assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    ModuleRoot roots[] = {capture.roots[1], capture.roots[2]};
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&arena, &observed.image, &files, NULL, NULL, 0, roots, 2, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    FILE *request = fopen("generated/context-supplier.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
    SourceCompilerClient compiler; source_compiler_client_init(&compiler, sources.allocator);
    assert(source_compiler_client_run(&compiler, "../../../../../../ama", "generated/context-supplier.json", &final.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyContextIssue context;
    SourceCopyContextStatus provider_status = source_copy_context_omissions_check(&omissions, &observed, &final, NULL, 0, &context);
    assert(provider_status == SOURCE_COPY_CONTEXT_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&arena, &analysis, &observed, &mains, &files, NULL, &omissions,
        &final, &compiler, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    assert(access("generated/project/packages/vendor/pkg/v1.0.0/src/parent", F_OK) == 0);
    source_compiler_client_destroy(&compiler); source_image_capture_destroy(&final); source_observed_image_destroy(&observed);
    source_file_map_destroy(&files); source_copy_context_omissions_destroy(&omissions); source_clone_map_destroy(&mains);
    source_arena_destroy(&arena); finish_copy();
    destroy_supplier_context_files();
}
