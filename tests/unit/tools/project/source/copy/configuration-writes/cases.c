/** @file
 * @brief Check existing and initialized configuration recipes against exact expected bytes and real compilation.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Uncovered retained targets need precisely one actual concrete permission in
 * extend mode; a full cloned closure needs no external permission. */
static void it_prepares_owned_existing_and_initialized_configuration_writes_from_real_copy_facts(void) {
    for (unsigned profile = 0; profile < 5; profile++) {
        bool initialized = profile >= 2, cloned = profile == 3, covered = profile == 1;
        SourceCopyDependencyMode mode = profile == 4 ? SOURCE_COPY_KEEP_EXTERNAL
            : cloned ? SOURCE_COPY_CLONE_DEPENDENCIES : SOURCE_COPY_EXTEND_PACKAGE;
        PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, mode, covered, 0, false, initialized);
        ConfigurationWriteFixture material; configuration_write_fixture(&material, &fixture);
        SourceOperationConfigurationWrite write; source_operation_configuration_write_init(&write, sources.allocator);
        SourceOperationConfigurationWriteStatus status = source_operation_configuration_write_prepare(&write, &material.inputs);
        if (profile == 4) {
            assert(status == SOURCE_OPERATION_CONFIGURATION_PERMISSION && !write.valid
                && write.configuration_issue.permission_status == SOURCE_PERMISSION_UNDECLARED);
        } else {
            assert(status == SOURCE_OPERATION_CONFIGURATION_OK && write.valid && write.result);
            const char *expected = cloned ? "" : "[dependencies]\n/vendor/pkg/v1.0.0/src/other\n";
            assert(source_length(write.result) == strlen(expected) && !memcmp(source_bytes(write.result), expected, strlen(expected)));
            assert(write.roots.count == (cloned ? 0u : 1u)); verify_configuration_write(&fixture, &write, &material);
            size_t before = allocation_count;
            assert(source_operation_configuration_write_prepare(&write, &material.inputs) == SOURCE_OPERATION_CONFIGURATION_INVALID);
            assert(write.valid && allocation_count == before);
        }
        source_operation_configuration_write_destroy(&write); configuration_write_fixture_destroy(&material); finish_permission_additions(&fixture);
    }
}
