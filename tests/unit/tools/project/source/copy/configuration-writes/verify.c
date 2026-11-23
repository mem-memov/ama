/** @file
 * @brief Independently reparse actual authorized configuration output and compile its complete copied graph.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Compare the actual emitted bytes/roots, never re-invoke the writer as oracle. */
static void verify_configuration_write(PermissionAdditionFixture *fixture, const SourceOperationConfigurationWrite *write, const ConfigurationWriteFixture *material) {
    const SourceOperationConfigurationWrite *configurations[] = {write};
    const SourceVersionInitialization *initializations[] = {&material->initialized};
    SourceOperationWritesInputs inputs = {.copy = &copy, .observed = &fixture->observed, .mains = &fixture->map,
        .files = &material->files, .main_texts = fixture->provenance, .configurations = configurations, .configuration_count = 1,
        .initializations = fixture->new_version ? initializations : NULL, .initialization_count = fixture->new_version ? 1u : 0u};
    SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
    assert(source_operation_writes_build(&writes, &inputs) == SOURCE_OPERATION_WRITES_OK && writes.valid);
    check_source_write_join_failures(&inputs);
    const SourceOperationConfigurationWrite *duplicates[] = {write, write};
    SourceOperationWritesInputs bad = inputs; bad.configurations = duplicates; bad.configuration_count = 2;
    SourceOperationWrites refused; source_operation_writes_init(&refused, sources.allocator);
    assert(source_operation_writes_build(&refused, &bad) == SOURCE_OPERATION_WRITES_DUPLICATE && !refused.valid);
    source_operation_writes_destroy(&refused);
    if (fixture->new_version) {
        bad = inputs; bad.initializations = NULL; bad.initialization_count = 0;
        source_operation_writes_init(&refused, sources.allocator);
        assert(source_operation_writes_build(&refused, &bad) == SOURCE_OPERATION_WRITES_SCOPE && !refused.valid);
        source_operation_writes_destroy(&refused);
    }
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&fixture->arena, &fixture->observed.image, &material->files, writes.prepared,
        writes.changes, writes.count, capture.roots, capture.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    FILE *request = fopen("generated/authorized-configuration.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/authorized-configuration.json", &final.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&fixture->arena, &analysis, &fixture->observed, &fixture->map, &material->files,
        writes.prepared, &material->contexts, &final, &client, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    check_complete_retained_roots(&inputs, &material->contexts, &client, SOURCE_OPERATION_SCOPE_WRITER);
    if (fixture->cloned && fixture->new_version) { check_complete_additional_placement_refusal(fixture, material); }
    SourceSitesIssue sites;
    assert(source_graph_sites(analysis.original.response.definitions, client.response.definitions, &fixture->map,
        NULL, 0, (SourceRegistryRetirement){0}, fixture->provenance, &sites) == SOURCE_SITES_OK);
    SourcePackageEnvironment environment; source_package_environment_init(&environment, sources.allocator);
    assert(source_package_environment_build(&environment, &final.catalog) == SOURCE_ENVIRONMENT_OK);
    const SourcePackageScope *actual = environment_scope(&environment, fixture->target_identity);
    SourceConfigurationCheckIssue issue;
    assert(source_copy_configuration_check(&fixture->final, fixture->scope, &write->roots, &environment, actual, &issue) == SOURCE_CONFIGURATION_CHECK_OK);
    source_package_environment_destroy(&environment); source_compiler_client_destroy(&client); source_image_capture_destroy(&final); source_operation_writes_destroy(&writes);
}
