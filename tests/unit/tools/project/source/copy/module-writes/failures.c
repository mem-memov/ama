/** @file
 * @brief Exhaust module-alias write allocations and reject foreign maps, missing scope and unowned INI material.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** The actual parser/normalizer/writer and source-text failed prefixes remain
 * destructible; no original byte/scope/map changes on refusal. */
static void it_retires_module_alias_write_failures_and_refuses_foreign_companion_authority(void) {
    for (unsigned standard = 0; standard < 2; standard++) {
        ModuleWriteFixture fixture; module_write_fixture(&fixture, standard != 0);
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            SourceOperationModuleWrite write; source_operation_module_write_init(&write, sources.allocator);
            size_t before = allocation_count; if (failure) { failure_at = before + failure; }
            SourceOperationModuleWriteStatus status = source_operation_module_write_prepare(&write, &fixture.inputs);
            if (!failure) { assert(status == SOURCE_OPERATION_MODULE_OK); total = allocation_count - before; assert(total); }
            else { assert(status != SOURCE_OPERATION_MODULE_OK && !write.valid && !write.result); }
            failure_at = 0; source_operation_module_write_destroy(&write); assert(live_allocations == baseline);
        }
        SourceFileMap empty; source_file_map_init(&empty, sources.allocator); assert(source_file_map_seal(&empty) == SOURCE_FILE_MAP_OK);
        for (unsigned profile = 0; profile < 3; profile++) {
            SourceOperationModuleWriteInputs inputs = fixture.inputs; SourceCloneMapping foreign = *fixture.clone;
            if (!profile) { inputs.clone = &foreign; }
            if (profile == 1) { inputs.files = &empty; }
            if (profile == 2) { inputs.copy = NULL; }
            SourceOperationModuleWrite write; source_operation_module_write_init(&write, sources.allocator);
            assert(source_operation_module_write_prepare(&write, &inputs) != SOURCE_OPERATION_MODULE_OK && !write.valid && !write.result);
            source_operation_module_write_destroy(&write);
        }
        source_file_map_destroy(&empty);
        if (!standard) {
            SourceOperationModuleWriteInputs inputs = fixture.inputs; inputs.original = NULL;
            SourceOperationModuleWrite write; source_operation_module_write_init(&write, sources.allocator);
            assert(source_operation_module_write_prepare(&write, &inputs) == SOURCE_OPERATION_MODULE_SCOPE && !write.valid);
            source_operation_module_write_destroy(&write);
            for (const SourceCloneMapping *row = fixture.mains.first; row; row = row->next) {
                if (row == fixture.clone) { continue; }
                inputs = fixture.inputs; inputs.clone = row;
                source_operation_module_write_init(&write, sources.allocator);
                assert(source_operation_module_write_prepare(&write, &inputs) == SOURCE_OPERATION_MODULE_SCOPE && !write.valid);
                source_operation_module_write_destroy(&write);
            }
            SourceCloneMap draft; source_clone_map_init(&draft, sources.allocator, &capture);
            const ModuleSource *parent = NULL; const SourceCloneMapping *row = NULL;
            assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/parent", &parent) == MODULE_OK);
            const char *path = source_arena_join(&fixture.arena, parent->version->physical_path, "/", "src/copied draft"); assert(path);
            assert(source_clone_map_add(&draft, parent, path, "/vendor/pkg/v1.0.0/src/copied draft", &row) == SOURCE_CLONE_OK);
            assert(source_clone_map_seal(&draft) == SOURCE_CLONE_OK);
            inputs = fixture.inputs; inputs.mains = &draft; inputs.clone = row;
            source_operation_module_write_init(&write, sources.allocator);
            assert(source_operation_module_write_prepare(&write, &inputs) == SOURCE_OPERATION_MODULE_DRAFT && !write.valid);
            source_operation_module_write_destroy(&write); source_clone_map_destroy(&draft);
        }
        assert(source_operation_module_write_prepare(NULL, &fixture.inputs) == SOURCE_OPERATION_MODULE_INVALID);
        module_write_fixture_destroy(&fixture, standard != 0);
    }
}
