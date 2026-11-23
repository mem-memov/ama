/** @file
 * @brief Reject a genuinely prepared and independently compilable self-copy image.
 * @see [T276 checkpoint](../../../../../../../docs/implementation/source-operations.md#t276-remaining-implementation-steps-after-t283)
 */
/** Image preparation and ordinary compilation cannot authorize copying a raw
 * directory into itself; the operation intent gate must reject that request. */
static void it_refuses_self_overlap_even_when_the_prepared_image_compiles(void) {
    DomainFixture fixture; domain_begin(&fixture, 0);
    const char *version = domain_path(&fixture, fixture.project, "dependencies/vendor/pkg/v1.0.0");
    const char *source = domain_path(&fixture, version, "src"), *destination = domain_path(&fixture, source, "nested");
    SourceTreeSelection selected; source_tree_selection_init(&selected, sources.allocator);
    assert(source_tree_selection_read(&selected, &fixture.original, source) == SOURCE_TREE_OK);
    SourceCopyClosure copy; source_copy_closure_init(&copy, sources.allocator, &fixture.original, &fixture.analysis, NULL);
    assert(source_copy_closure_build(&copy, &selected, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &selected, destination) == SOURCE_PLACEMENT_OK);
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    assert(source_file_map_tree(&files, &placement) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, &fixture.original);
    const char *old[] = {"/vendor/pkg/v1.0.0/src/main", "/vendor/pkg/v1.0.0/src/helper",
        "/vendor/pkg/v1.0.0/tests/main/check", "/vendor/pkg/v1.0.0/tests/helper/check"};
    const char *suffixes[] = {"src/nested/main", "src/nested/helper", "tests/nested/main/check", "tests/nested/helper/check"};
    for (size_t i = 0; i < 4; i++) {
        const ModuleSource *unit = NULL;
        assert(module_catalog_find(&fixture.original.catalog, old[i], &unit) == MODULE_OK);
        const char *path = domain_path(&fixture, version, suffixes[i]); SourcePlacedIdentity identity;
        assert(source_placement_identity(&fixture.arena, fixture.original.roots, fixture.original.root_count,
            path, MODULE_PATH_MODULE, &identity) == SOURCE_IDENTITY_OK);
        const SourceCloneMapping *row = NULL;
        assert(source_clone_map_add(&mains, unit, path, identity.identity, &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
    SourceCopyContextOmissions contexts; source_copy_context_omissions_init(&contexts, sources.allocator, &copy, &mains);
    assert(source_copy_context_omissions_seal(&contexts) == SOURCE_COPY_CONTEXT_OK);
    SourcePackageEnvironment expected; source_package_environment_init(&expected, sources.allocator);
    assert(source_package_environment_build(&expected, &fixture.original.catalog) == SOURCE_ENVIRONMENT_OK);
    assert(expected.first && !expected.first->next && !expected.first->selection->count);
    SourceOperationConfigurationExpectation configuration = {.environment = &expected, .scope = expected.first};
    SourceOperationPlanInputs inputs = {.original = &fixture.analysis,
        .final_roots = fixture.original.roots, .final_root_count = fixture.original.root_count,
        .copy = &copy, .observed = &fixture.observed, .mains = &mains, .files = &files, .contexts = &contexts,
        .configurations = &configuration, .configuration_count = 1};
    SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
    assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_OK);
    /* This preliminary image deliberately lacks copied test outputs: it is
     * neither a complete copy proof nor a sealed operation. All represented
     * modules nevertheless pass the real ordinary compiler independently. */
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    domain_compile(&client, &plan.capture.view, "generated/domain-overlap.json");
    SourceOperationIntent intent = {.kind = SOURCE_OPERATION_COPY, .source = &selected, .placement = &placement};
    SourceOperationIntentIssue issue;
    assert(source_operation_intent_check(&plan, &intent, &issue) == SOURCE_OPERATION_INTENT_OVERLAP);
    assert(source_image_below(issue.path, source));
    source_compiler_client_destroy(&client); source_operation_plan_destroy(&plan);
    source_package_environment_destroy(&expected); source_copy_context_omissions_destroy(&contexts);
    source_clone_map_destroy(&mains); source_file_map_destroy(&files);
    source_tree_placement_destroy(&placement); source_copy_closure_destroy(&copy);
    source_tree_selection_destroy(&selected); domain_end(&fixture);
}
