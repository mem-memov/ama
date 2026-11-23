/** @file
 * @brief Exhaust authorized configuration owner allocations and refuse unrelated scope/category authority.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Every recipe failure retains destructible scratch without mutating its
 * original/final environments or real P/C owners. */
static void it_retires_configuration_write_prefixes_and_refuses_unowned_configuration_recipes(void) {
    for (unsigned initialized = 0; initialized < 2; initialized++) {
        PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, false, 0, false, initialized != 0);
        ConfigurationWriteFixture material; configuration_write_fixture(&material, &fixture);
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            SourceOperationConfigurationWrite write; source_operation_configuration_write_init(&write, sources.allocator);
            size_t before = allocation_count; if (failure) { failure_at = before + failure; }
            SourceOperationConfigurationWriteStatus status = source_operation_configuration_write_prepare(&write, &material.inputs);
            if (!failure) { assert(status == SOURCE_OPERATION_CONFIGURATION_OK && write.valid); total = allocation_count - before; assert(total); }
            else { assert(status != SOURCE_OPERATION_CONFIGURATION_OK && !write.valid && !write.result); }
            failure_at = 0; source_operation_configuration_write_destroy(&write); assert(live_allocations == baseline);
        }
        for (unsigned profile = 0; profile < 5; profile++) {
            SourceOperationConfigurationWriteInputs inputs = material.inputs;
            SourcePackageScope foreign = *fixture.scope;
            if (!profile) { inputs.after = &foreign; }
            if (profile == 1) { inputs.kind = SOURCE_OPERATION_CONFIGURATION_CARRIED; inputs.original = &fixture.original; inputs.before = environment_scope(&fixture.original, "/vendor/pkg/v1.0.0"); inputs.initialization = NULL; }
            if (profile == 2) { inputs.observed = NULL; }
            if (profile == 3) { inputs.files = NULL; }
            if (profile == 4) { inputs.kind = SOURCE_OPERATION_CONFIGURATION_EXISTING; inputs.original = &fixture.original; inputs.before = environment_scope(&fixture.original, "/vendor/pkg/v1.0.0"); inputs.initialization = NULL; }
            SourceOperationConfigurationWrite write; source_operation_configuration_write_init(&write, sources.allocator);
            assert(source_operation_configuration_write_prepare(&write, &inputs) != SOURCE_OPERATION_CONFIGURATION_OK && !write.valid && !write.result);
            source_operation_configuration_write_destroy(&write); assert(live_allocations == baseline);
        }
        assert(source_operation_configuration_write_prepare(NULL, &material.inputs) == SOURCE_OPERATION_CONFIGURATION_INVALID);
        configuration_write_fixture_destroy(&material); finish_permission_additions(&fixture);
    }
}
