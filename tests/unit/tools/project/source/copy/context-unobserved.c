/** @file
 * @brief Reject unknown context-destination absence using two genuine observers of one original domain.
 * @see [T276 resumed review](../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** An ignored directory name is not evidence about its children. Independently
 * prepare the complete final image, then require the operation's original facts
 * to prove that its new structural main was absent. */
static void it_refuses_unknown_context_destinations_without_borrowing_final_image_absence(void) {
    prepare_supplier_context(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    const char *external = source_arena_join(&arena, capture.roots[2].path, "/", "dependencies/vendor/pkg/v1.0.0"); assert(external);
    SourceObservationRequest requests[7];
    const char *suffixes[] = {"", "/package.ini", "/src", "/tests", "/src/main", "/src/other", "/src/unobserved"};
    for (size_t i = 0; i < 7; i++) {
        const char *path = source_arena_join(&arena, external, "", suffixes[i]); assert(path);
        requests[i] = (SourceObservationRequest){path, i == 1 || i == 4 || i == 5 ? SOURCE_OBSERVE_CONTENT : SOURCE_OBSERVE_CHILDREN, false};
    }
    SourceObservedImage complete, incomplete;
    source_observed_image_init(&complete, sources.allocator); source_observed_image_init(&incomplete, sources.allocator);
    assert(source_observed_image_build(&complete, &capture, (SourceImageBounds){0}, requests, 7) == SOURCE_OBSERVATION_OK);
    assert(source_observed_image_build(&incomplete, &capture, (SourceImageBounds){0}, requests, 6) == SOURCE_OBSERVATION_OK);
    SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, &capture);
    const char *names[] = {"/vendor/pkg/v1.0.0/src/parent", child_identity, "/vendor/pkg/v1.0.0/tests/parent/child/check"};
    const char *destinations[] = {"/src/unobserved/needed", "/src/unobserved/needed+/copied", "/tests/unobserved/needed/copied/check"};
    const char *identities[] = {"/vendor/pkg/v1.0.0/src/unobserved/needed", "/vendor/pkg/v1.0.0/src/unobserved/needed/copied", "/vendor/pkg/v1.0.0/tests/unobserved/needed/copied/check"};
    for (size_t i = 0; i < 3; i++) {
        const ModuleSource *source = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, names[i], &source) == MODULE_OK);
        const char *path = source_arena_join(&arena, external, "", destinations[i]); assert(path);
        assert(source_clone_map_add(&mains, source, path, identities[i], &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
    SourceCopyContextOmissions omissions; source_copy_context_omissions_init(&omissions, sources.allocator, &copy, &mains);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&files, &copy, &mains, &omissions, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_context_omissions_seal(&omissions) == SOURCE_COPY_CONTEXT_OK);
    SourceVersionPlacement version = {"/vendor/pkg/v1.0.0", external}; SourceCopyTestsIssue tests;
    assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    ModuleRoot roots[] = {capture.roots[1], capture.roots[2]};
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&arena, &complete.image, &files, NULL, NULL, 0, roots, 2, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    FILE *request = fopen("generated/context-unknown.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
    SourceCompilerClient compiler; source_compiler_client_init(&compiler, sources.allocator);
    assert(source_compiler_client_run(&compiler, "../../../../../../ama", "generated/context-unknown.json", &final.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyContextIssue context;
    assert(source_copy_context_copies_check(&omissions, &complete, &final, &context) == SOURCE_COPY_CONTEXT_OK);
    assert(source_copy_context_copies_check(&omissions, &incomplete, &final, &context) == SOURCE_COPY_CONTEXT_UNOBSERVED);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&arena, &analysis, &complete, &mains, &files, NULL, &omissions,
        &final, &compiler, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    assert(source_copy_prove(&arena, &analysis, &incomplete, &mains, &files, NULL, &omissions,
        &final, &compiler, NULL, 0, &proof) == SOURCE_COPY_PROOF_CONTEXT);
    source_compiler_client_destroy(&compiler); source_image_capture_destroy(&final); source_observed_image_destroy(&complete);
    source_observed_image_destroy(&incomplete); source_file_map_destroy(&files);
    source_copy_context_omissions_destroy(&omissions); source_clone_map_destroy(&mains); source_arena_destroy(&arena);
    finish_copy(); destroy_supplier_context_files();
}
