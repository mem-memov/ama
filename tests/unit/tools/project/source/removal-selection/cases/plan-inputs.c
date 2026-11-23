/** @file
 * @brief Check removal-demand atomicity and shared-plan original/copy/deletion admission.
 * @see [T283 contract](../../../../../../../docs/implementation/source-operations.md#t283-entry-review-and-execution)
 */
/** Requests and removals preserve every actual selected key once; invalid inputs
 * and either allocation failure leave the caller's published result unchanged. */
static void it_prepares_exact_removal_demands_and_retires_failed_outputs(void) {
    begin_removal(MATERIAL_NONE, main_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    SourceRemovalMaterial output = {.count = SIZE_MAX};
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    assert(source_removal_material_prepare(NULL, &removal, &output) == SOURCE_REMOVAL_MATERIAL_INVALID);
    assert(source_removal_material_prepare(&arena, NULL, &output) == SOURCE_REMOVAL_MATERIAL_INVALID);
    assert(source_removal_material_prepare(&arena, &removal, NULL) == SOURCE_REMOVAL_MATERIAL_INVALID);
    assert(output.count == SIZE_MAX);
    source_arena_destroy(&arena);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_arena_init(&arena, sources.allocator); size_t before = allocation_count;
        output = (SourceRemovalMaterial){.count = SIZE_MAX};
        if (failure) { failure_at = before + failure; }
        SourceRemovalMaterialStatus status = source_removal_material_prepare(&arena, &removal, &output);
        if (!failure) {
            assert(status == SOURCE_REMOVAL_MATERIAL_OK && output.count == removal.path_count);
            total = allocation_count - before; assert(total);
            size_t index = 0;
            for (const SourceRemovalSelectedFile *row = removal.selected; row; row = row->next, index++) {
                assert(output.requests[index].path == row->file->file.path);
                assert(output.requests[index].kind == (row->file->directory ? SOURCE_OBSERVE_CHILDREN : SOURCE_OBSERVE_CONTENT));
                assert(output.changes[index].kind == SOURCE_IMAGE_REMOVE && output.changes[index].entry.path == row->file->file.path);
            }
            assert(index == output.count);
        } else { assert(status == SOURCE_REMOVAL_MATERIAL_MEMORY && output.count == SIZE_MAX); }
        failure_at = 0; source_arena_destroy(&arena); assert(live_allocations == baseline);
    }
    end_removal(MATERIAL_NONE);
}
/** Deletion cannot smuggle copied material or context obligations through absent
 * copy provenance, and every plan still needs its admitted original analysis. */
static void it_refuses_mixed_copy_deletion_plan_owners(void) {
    begin_removal(MATERIAL_NONE, main_path);
    assert(source_removal_selection_build(&removal, &selection, &analysis, NULL) == SOURCE_REMOVAL_SELECTION_OK);
    RemovalPlanFixture fixture; assert(removal_plan_fixture_build(&fixture, &removal));
    SourceFileMap nonempty; source_file_map_init(&nonempty, sources.allocator);
    const SourceFileMapping *mapped = NULL;
    assert(source_file_map_add(&nonempty, removal.selected->file, "/outside", &mapped) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&nonempty) == SOURCE_FILE_MAP_OK);
    for (unsigned profile = 0; profile < 5; profile++) {
        SourceOperationPlanInputs inputs = fixture.plan.inputs;
        SourceCopyContextOmissions invalid_context = {0};
        if (!profile) { inputs.original = NULL; }
        if (profile == 1) { inputs.removal = NULL; }
        if (profile == 2) { inputs.contexts = &invalid_context; }
        if (profile == 3) { inputs.files = &nonempty; }
        if (profile == 4) { inputs.observed = NULL; }
        SourceOperationPlan plan; source_operation_plan_init(&plan, sources.allocator);
        assert(source_operation_plan_build(&plan, &inputs) == SOURCE_OPERATION_PLAN_INVALID && !plan.prepared);
        source_operation_plan_destroy(&plan);
    }
    source_file_map_destroy(&nonempty); removal_plan_fixture_destroy(&fixture); end_removal(MATERIAL_NONE);
}
