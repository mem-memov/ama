/** @file
 * @brief Reject a genuinely compiling clone whose additional dependency suffix violates operation placement.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Recreate the former permission fixture's arbitrary other-to-helper rename as
 * an intentional negative operation. Independent compilation and source/site
 * preservation must succeed; exact request/material reconstruction must refuse. */
static void check_complete_additional_placement_refusal(const PermissionAdditionFixture *fixture,
    const ConfigurationWriteFixture *material) {
    assert(fixture->cloned && fixture->new_version);
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceManager texts; source_manager_init(&texts, sources.allocator);
    SourceCloneMap mains; source_clone_map_init(&mains, sources.allocator, &capture);
    for (const SourceCloneMapping *old = fixture->map.first; old; old = old->next) {
        const char *suffix = !strcmp(old->original->logical_path, "/vendor/pkg/v1.0.0/src/other") ? "/src/helper"
            : !strcmp(old->original->logical_path, "/vendor/pkg/v1.0.0/tests/other/check") ? "/tests/helper/check" : NULL;
        const char *identity = suffix ? source_arena_join(&arena, fixture->target_identity, "", suffix) : old->identity;
        const char *path = suffix ? source_arena_join(&arena, fixture->target_physical, "", suffix) : old->path;
        const SourceCloneMapping *row = NULL; assert(identity && path);
        assert(source_clone_map_add(&mains, old->original, path, identity, &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&mains) == SOURCE_CLONE_OK);
    SourceCopyContextOmissions contexts; source_copy_context_omissions_init(&contexts, sources.allocator, &copy, &mains);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&files, &copy, &mains, &contexts, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_context_omissions_seal(&contexts) == SOURCE_COPY_CONTEXT_OK);
    SourceVersionPlacement version = {fixture->target_identity, fixture->target_physical}; SourceCopyTestsIssue tests;
    assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    SourceTextEdits edits[2]; SourceGraphPreparedText prepared[2]; size_t count = 0;
    for (const SourceCloneMapping *row = mains.first; row; row = row->next) {
        const DeliverySourceDefinition *definition = permission_definition(row->original->logical_path);
        if (!definition->dependencies) { continue; }
        assert(count < 2); source_text_edits_init(&edits[count], sources.allocator, definition->dependencies->origin.source);
        for (const DeliverySourceDependency *edge = definition->dependencies; edge; edge = edge->next) {
            SourceOperandIssue issue; assert(source_operand_rooted(&edits[count], &mains, edge, &issue) == SOURCE_OPERAND_OK);
        }
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edits[count], &texts, row->path, &result) == SOURCE_EDITS_OK);
        prepared[count] = (SourceGraphPreparedText){.edits = &edits[count]}; count++;
    }
    assert(count == 2); prepared[0].next = &prepared[1];
    const SourceVersionInitialization *initializations[] = {&material->initialized};
    SourceOperationWritesInputs write_inputs = {.copy = &copy, .observed = &fixture->observed,
        .mains = &mains, .files = &files, .main_texts = prepared, .initializations = initializations, .initialization_count = 1};
    SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
    assert(source_operation_writes_build(&writes, &write_inputs) == SOURCE_OPERATION_WRITES_OK);
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    assert(source_copy_image_prepare(&arena, &fixture->observed.image, &files, writes.prepared, writes.changes,
        writes.count, capture.roots, capture.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
    SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
    assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
    FILE *request = fopen("generated/additional-placement.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
    SourceCompilerClient compiler; source_compiler_client_init(&compiler, sources.allocator);
    assert(source_compiler_client_run(&compiler, "../../../../../../ama", "generated/additional-placement.json", &final.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&arena, &analysis, &fixture->observed, &mains, &files, writes.prepared,
        &contexts, &final, &compiler, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    const SourceFileMapping *raw = files.first;
    while (raw && strcmp(raw->source, tree.operand.path)) { raw = raw->next; } assert(raw);
    SourceTreePlacement placement; source_tree_placement_init(&placement, sources.allocator);
    assert(source_tree_placement_build(&placement, &tree, raw->destination) == SOURCE_PLACEMENT_OK);
    SourceOperationIntent intent = {.kind = SOURCE_OPERATION_COPY, .source = &tree, .placement = &placement};
    SourceOperationRootDisposition roots[2] = {{SOURCE_OPERATION_ROOT_KEEP, 0}, {SOURCE_OPERATION_ROOT_KEEP, 1}};
    SourceOperationCompleteInputs inputs = {.writes = &write_inputs, .intent = &intent, .original = &analysis,
        .configured = &capture, .final_roots = capture.roots, .final_root_count = capture.root_count,
        .contexts = &contexts, .root_originals = roots, .root_original_count = capture.root_count};
    SourceOperationComplete owner; source_operation_complete_init(&owner, sources.allocator);
    assert(source_operation_complete_prepare(&owner, &inputs) == SOURCE_OPERATION_COMPLETE_MATERIAL
        && owner.material.status == SOURCE_OPERATION_MATERIAL_LOGICAL && !owner.prepared && !owner.sealed);
    assert(owner.material.failed_main && owner.material.failed_path);
    const char *bad_source = owner.material.failed_main->logical_path;
    const char *expected = !strcmp(bad_source, "/vendor/pkg/v1.0.0/src/other") ? "/target/new/v3.0.0/src/other"
        : !strcmp(bad_source, "/vendor/pkg/v1.0.0/tests/other/check") ? "/target/new/v3.0.0/tests/other/check" : NULL;
    assert(expected && !strcmp(owner.material.failed_path, expected));
    source_operation_complete_destroy(&owner); source_tree_placement_destroy(&placement);
    source_compiler_client_destroy(&compiler); source_image_capture_destroy(&final); source_operation_writes_destroy(&writes);
    for (size_t i = 0; i < count; i++) { source_text_edits_destroy(&edits[i]); }
    source_file_map_destroy(&files); source_copy_context_omissions_destroy(&contexts); source_clone_map_destroy(&mains);
    source_manager_destroy(&texts); source_arena_destroy(&arena);
}
