/** @file
 * @brief Pass actual promotion/relocation through the full copy proof and reject ignored-source loss.
 * @see [T275 contract](../../../../../../../docs/implementation/source-operations.md#t275-entry-review-and-execution)
 */
/** An independently compiling selected view does not authorize deleting or
 * modifying a retired external original that still exists in the physical plan. */
static void domain_refuse_lost_original(DomainFixture *fixture, const SourceFileMap *files,
    const SourceCopyContextOmissions *contexts) {
    const char *path = source_path(fixture->original.units->source->text);
    for (unsigned edited = 0; edited < 4; edited++) {
        size_t count = fixture->image.count, used = 0;
        SourceCatalogImageEntry *rows = source_arena_allocate(&fixture->arena, (count + 1) * sizeof(*rows)); assert(rows);
        bool found = false; struct stat main_metadata = {0};
        for (size_t i = 0; i < count; i++) {
            SourceCatalogImageEntry row = fixture->image.entries[i];
            if (!strcmp(row.path, path)) {
                found = true; main_metadata = row.metadata;
                if (!edited) { continue; }
                if (edited == 1) {
                    const unsigned char changed[] = "changed ignored original";
                    assert(source_manager_add(&sources, "changed-ignored", changed, sizeof(changed)-1, &row.content) == SOURCE_OK);
                }
            }
            if (edited == 3) {
                const char *slash = strrchr(path, '/'); size_t length = (size_t)(slash - path);
                if (strlen(row.path) == length && !memcmp(row.path, path, length)) { row.children_complete = false; }
            }
            rows[used++] = row;
        }
        if (edited == 2) {
            const char *extra = source_arena_join(&fixture->arena, path, ".", "extra"); assert(extra);
            const SourceText *bytes = NULL;
            assert(source_manager_add(&sources, extra, (const unsigned char *)"extra", 5, &bytes) == SOURCE_OK);
            main_metadata.st_dev = 0; main_metadata.st_ino = 100000;
            rows[used++] = (SourceCatalogImageEntry){.path = extra, .metadata = main_metadata,
                .metadata_captured = true, .content = bytes};
        }
        assert(found); SourceCatalogImage image; SourceImageIssue shape;
        assert(source_catalog_image_prepare(&image, rows, used, fixture->image.roots, fixture->image.root_count,
            fixture->image.bounds, &shape) == SOURCE_IMAGE_OK);
        SourceImageCapture captured; source_image_capture_init(&captured, sources.allocator);
        assert(source_image_capture_build(&captured, &image) == SOURCE_IMAGE_CAPTURE_OK);
        SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
        domain_compile(&client, &captured.view, "generated/domain-bad.json");
        SourceCopyProofIssue issue;
        assert(source_copy_prove(&fixture->arena, &fixture->analysis, &fixture->map, files,
            fixture->prepared, contexts, &captured, &client, NULL, 0, &issue) == SOURCE_COPY_PROOF_ORIGINAL);
        assert(!strcmp(issue.path, path));
        source_compiler_client_destroy(&client); source_image_capture_destroy(&captured);
    }
}
/** Compose genuine raw selection, test closure, complete physical mapping and
 * context accounting rather than constructing fake valid proof owners. */
static void domain_prove(DomainFixture *fixture) {
    SourceTreeSelection selected; source_tree_selection_init(&selected, sources.allocator);
    assert(source_tree_selection_read(&selected, &fixture->original, fixture->prefix) == SOURCE_TREE_OK);
    SourceCopyClosure copied; source_copy_closure_init(&copied, sources.allocator, &fixture->original, &fixture->analysis, NULL);
    assert(source_copy_closure_build(&copied, &selected, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    for (const AcquisitionUnitEntry *row = selected.members; row; row = row->next) {
        const SourceFileMapping *mapped = NULL;
        assert(source_file_map_add(&files, row, domain_mapped_path(fixture, row->file.path), &mapped) == SOURCE_FILE_MAP_OK);
    }
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    SourceImageChange replacement; size_t replacements = 0;
    for (size_t i = 0; i < fixture->observed.image.count; i++) {
        if (!strcmp(fixture->observed.image.entries[i].path, fixture->destination)) {
            replacement = (SourceImageChange){SOURCE_IMAGE_REMOVE, fixture->observed.image.entries[i]}; replacements = 1;
        }
    }
    SourceCopyContextOmissions contexts; source_copy_context_omissions_init(&contexts, sources.allocator, &copied, &fixture->map);
    assert(source_copy_context_omissions_seal(&contexts) == SOURCE_COPY_CONTEXT_OK);
    SourcePackageEnvironment expected_environment; source_package_environment_init(&expected_environment, sources.allocator);
    assert(source_package_environment_build(&expected_environment, &fixture->final.catalog) == SOURCE_ENVIRONMENT_OK);
    /* This independent reference image has one empty package.ini. An expected
     * scope proves its exact location/configuration, not inferred permissions. */
    assert(expected_environment.first && !expected_environment.first->next);
    SourceCopyConfigurationExpectation expectation = {.environment = &expected_environment, .scope = expected_environment.first};
    SourceTreeSelection replaced_tree; source_tree_selection_init(&replaced_tree, sources.allocator);
    SourceRemovalSelection removed; source_removal_selection_init(&removed, sources.allocator);
    if (replacements) {
        assert(source_tree_selection_read(&replaced_tree, &fixture->original, fixture->destination) == SOURCE_TREE_OK);
        assert(source_removal_selection_build(&removed, &replaced_tree, &fixture->analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
        assert(removed.path_count == 1 && removed.identity_count == 0);
    }
    SourceCopyPlanInputs inputs = {.final_roots = &fixture->final_root, .final_root_count = 1,
        .copy = &copied, .observed = &fixture->observed, .mains = &fixture->map, .files = &files,
        .texts = fixture->prepared, .contexts = &contexts,
        .changes = replacements ? &replacement : NULL, .change_count = replacements,
        .removal = replacements ? &removed : NULL, .configurations = &expectation, .configuration_count = 1};
    SourceCopyPlan plan; source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_OK);
    assert(plan.image->roots == &fixture->final_root && plan.image->root_count == 1);
    assert(delivery_source_response_matches(&fixture->after.response, &plan.capture.view));
    assert(plan.mutations.count == 1 && !strcmp(plan.mutations.roots[0].path, fixture->destination));
    SourceCopyProofIssue issue;
    assert(source_copy_plan_prove(&plan, &fixture->after, &issue) == SOURCE_COPY_PROOF_OK);
    domain_refuse_lost_original(fixture, &files, &contexts);
    source_copy_plan_destroy(&plan);
    inputs.final_roots = NULL;
    source_copy_plan_init(&plan, sources.allocator);
    assert(source_copy_plan_build(&plan, &inputs) == SOURCE_COPY_PLAN_INVALID && !plan.prepared);
    source_copy_plan_destroy(&plan);
    source_removal_selection_destroy(&removed); source_tree_selection_destroy(&replaced_tree);
    source_package_environment_destroy(&expected_environment);
    source_copy_context_omissions_destroy(&contexts); source_file_map_destroy(&files);
    source_copy_closure_destroy(&copied); source_tree_selection_destroy(&selected);
}
