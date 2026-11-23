/** @file
 * @brief Reject a genuinely constructed but unrelated empty-version write even though its whole final image compiles.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Valid initialization facts and ordinary compiler acceptance do not grant a
 * source operation permission to create an unrelated package version. */
static void it_refuses_unrelated_initialized_versions_despite_actual_final_compiler_acceptance(void) {
    PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, true, 0, false, false);
    ConfigurationWriteFixture material; configuration_write_fixture(&material, &fixture);
    const SourcePackageScope *scope = environment_scope(&fixture.original, "/vendor/pkg/v1.0.0");
    const SourceCatalogImageEntry *mode = NULL;
    for (size_t i = 0; i < fixture.observed.image.count; i++) {
        if (!strcmp(fixture.observed.image.entries[i].path, source_path(scope->document.source))) { mode = &fixture.observed.image.entries[i]; }
    }
    assert(mode); const char *path = source_arena_join(&fixture.arena, capture.roots[0].path, "/", "packages/vendor/unrelated/v9.0.0"); assert(path);
    SourceVersionPlacement version = {"/vendor/unrelated/v9.0.0", path};
    SourceVersionInitialization initialization; SourceVersionInitializationIssue initialized;
    assert(source_version_initialize(&fixture.arena, &sources, &fixture.observed, capture.roots, capture.root_count,
        &version, &material.files, mode, NULL, 0, &initialization, &initialized) == SOURCE_VERSION_INITIALIZATION_OK);
    assert(initialization.count == 3);
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&fixture.arena, &fixture.observed.image, &material.files, fixture.provenance,
        initialization.changes, initialization.count, capture.roots, capture.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    FILE *request = fopen("generated/unrelated-initialized-version.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/unrelated-initialized-version.json", &final.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&fixture.arena, &analysis, &fixture.observed, &fixture.map, &material.files,
        fixture.provenance, &material.contexts, &final, &client, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    const SourceVersionInitialization *initializations[] = {&initialization};
    SourceOperationWritesInputs inputs = {.copy = &copy, .observed = &fixture.observed, .mains = &fixture.map,
        .files = &material.files, .main_texts = fixture.provenance, .initializations = initializations, .initialization_count = 1};
    SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
    assert(source_operation_writes_build(&writes, &inputs) == SOURCE_OPERATION_WRITES_SCOPE && !writes.valid && !strcmp(writes.failed_path, path));
    source_operation_writes_destroy(&writes); source_compiler_client_destroy(&client); source_image_capture_destroy(&final);
    configuration_write_fixture_destroy(&material); finish_permission_additions(&fixture);
}
