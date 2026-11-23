/** @file
 * @brief Reject raw companion preparations, cyclic input, duplicate producers and impossible counts in the closed join.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Real applied INI text remains unauthorized as a main preparation. Every
 * invalid shape retains its real successful producer and frozen source owners. */
static void it_refuses_untyped_companion_routes_and_duplicate_or_unbounded_write_joins(void) {
    ModuleWriteFixture fixture; module_write_fixture(&fixture, false);
    SourceOperationModuleWrite write; source_operation_module_write_init(&write, sources.allocator);
    assert(source_operation_module_write_prepare(&write, &fixture.inputs) == SOURCE_OPERATION_MODULE_OK);
    const SourceOperationModuleWrite *modules[] = {&write, &write};
    SourceGraphPreparedText main = {.edits = &fixture.main_edits}, untyped = {.edits = &write.edits};
    SourceOperationWritesInputs baseline = {.copy = &copy, .observed = &fixture.observed, .mains = &fixture.mains,
        .files = &fixture.files, .main_texts = &main, .modules = modules, .module_count = 1};
    for (unsigned profile = 0; profile < 6; profile++) {
        SourceOperationWritesInputs inputs = baseline; SourceGraphPreparedText cyclic = main; cyclic.next = &cyclic;
        if (!profile) { inputs.main_texts = &untyped; inputs.modules = NULL; inputs.module_count = 0; }
        if (profile == 1) { inputs.main_texts = &cyclic; }
        if (profile == 2) { inputs.module_count = 2; }
        if (profile == 3) { inputs.module_count = SIZE_MAX; }
        if (profile == 4) { inputs.initialization_count = SIZE_MAX; inputs.initializations = (const SourceVersionInitialization *const[]){NULL}; }
        if (profile == 5) { inputs.configuration_count = SIZE_MAX; inputs.configurations = (const SourceOperationConfigurationWrite *const[]){NULL}; }
        SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
        SourceOperationWritesStatus expected = !profile ? SOURCE_OPERATION_WRITES_SCOPE : profile == 1 ? SOURCE_OPERATION_WRITES_INVALID
            : profile == 2 ? SOURCE_OPERATION_WRITES_DUPLICATE : SOURCE_OPERATION_WRITES_LIMIT;
        assert(source_operation_writes_build(&writes, &inputs) == expected && !writes.valid);
        source_operation_writes_destroy(&writes); assert(write.valid && write.edits.valid && fixture.mains.ready && fixture.files.ready);
    }
    assert(source_operation_writes_build(NULL, &baseline) == SOURCE_OPERATION_WRITES_INVALID);
    SourceOperationWrites writes; source_operation_writes_init(&writes, sources.allocator);
    assert(source_operation_writes_build(&writes, NULL) == SOURCE_OPERATION_WRITES_INVALID && !writes.valid);
    source_operation_writes_destroy(&writes); source_operation_module_write_destroy(&write); module_write_fixture_destroy(&fixture, false);
}
