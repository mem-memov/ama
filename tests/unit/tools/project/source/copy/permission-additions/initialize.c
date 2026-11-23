/** @file
 * @brief Verify initialized scopes use observed absence and genuine empty permission baselines.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
/** New destinations admit only mode-authorized edges, then actual final compiler
 * and C/site checks accept the independently reparsed concrete configuration. */
static void it_prepares_new_version_permissions_from_empty_scope_and_passes_actual_final_compiler(void) {
    const SourceCopyDependencyMode modes[] = {SOURCE_COPY_EXTEND_PACKAGE, SOURCE_COPY_CLONE_DEPENDENCIES, SOURCE_COPY_KEEP_EXTERNAL};
    for (size_t i = 0; i < 3; i++) {
        PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, modes[i], false, 0, false, true);
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, fixture.scope->document.source);
        SourceSelectionRoots output = {0}; SourceConfigurationIssue issue;
        SourceConfigurationStatus status = source_copy_configuration_initialize(&fixture.arena, &edits, &copy,
            &fixture.map, &fixture.observed, &fixture.final, fixture.scope, &output, &issue);
        if (i == 2) { assert(status == SOURCE_CONFIGURATION_PERMISSIONS && issue.permission_status == SOURCE_PERMISSION_UNDECLARED && !edits.first); }
        else {
            assert(status == SOURCE_CONFIGURATION_OK && output.count == (i == 0 ? 1 : 0));
            assert(!fixture.scope->aliases->count && !fixture.scope->selection->count);
            verify_permission_addition(&fixture, &output);
        }
        source_text_edits_destroy(&edits); finish_permission_additions(&fixture);
    }
}
/** Every failed allocation retires; existing directories, foreign owners and
 * nonempty provisional package files cannot be treated as new empty scopes. */
static void it_retires_initialized_scope_allocations_and_refuses_existing_or_foreign_environments(void) {
    PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, false, 0, false, true);
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, fixture.scope->document.source);
        SourceSelectionRoots output = fixture.baseline; SourceConfigurationIssue issue;
        size_t start = allocation_count; if (failure) { failure_at = start + failure; }
        SourceConfigurationStatus status = source_copy_configuration_initialize(&arena, &edits, &copy, &fixture.map,
            &fixture.observed, &fixture.final, fixture.scope, &output, &issue);
        if (!failure) { assert(status == SOURCE_CONFIGURATION_OK); total = allocation_count - start; assert(total); }
        else {
            assert((status == SOURCE_CONFIGURATION_PERMISSIONS && issue.permission_status == SOURCE_PERMISSION_MEMORY)
                || (status == SOURCE_CONFIGURATION_EDITS && issue.edits_status == SOURCE_EDITS_MEMORY));
            assert(output.entries == fixture.baseline.entries && output.count == fixture.baseline.count);
        }
        failure_at = 0; source_text_edits_destroy(&edits); source_arena_destroy(&arena); assert(live_allocations == baseline);
    }
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, fixture.scope->document.source);
    SourceSelectionRoots output = {0}; SourceConfigurationIssue issue; SourcePackageScope foreign = *fixture.scope;
    assert(source_copy_configuration_initialize(&fixture.arena, &edits, &copy, &fixture.map, &fixture.observed,
        &fixture.final, &foreign, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    SourceObservedImage later; source_observed_image_init(&later, sources.allocator);
    /* This is a distinct actual observer that now sees the directory existing. */
    file("generated/project/packages/target/new/v3.0.0/package.ini", "; unexpected prefilled scope\n");
    SourceRootCapture current; source_root_capture_init(&current, sources.allocator);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT}, {.path = "generated/standard", .kind = MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&current, roots, 2) == SOURCE_CAPTURE_OK);
    assert(source_observed_image_build(&later, &current, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    assert(source_copy_configuration_initialize(&fixture.arena, &edits, &copy, &fixture.map, &later,
        &fixture.final, fixture.scope, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    SourcePackageEnvironment nonempty; source_package_environment_init(&nonempty, sources.allocator);
    assert(source_package_environment_build(&nonempty, &current.catalog) == SOURCE_ENVIRONMENT_OK);
    const SourcePackageScope *changed = environment_scope(&nonempty, fixture.target_identity);
    SourceTextEdits changed_edits; source_text_edits_init(&changed_edits, sources.allocator, changed->document.source);
    assert(source_copy_configuration_initialize(&fixture.arena, &changed_edits, &copy, &fixture.map, &fixture.observed,
        &nonempty, changed, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    assert(!changed_edits.first); source_text_edits_destroy(&changed_edits); source_package_environment_destroy(&nonempty);
    source_observed_image_destroy(&later); source_root_capture_destroy(&current);
    source_text_edits_destroy(&edits); finish_permission_additions(&fixture);
    prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, false, 0, false, false);
    source_text_edits_init(&edits, sources.allocator, fixture.scope->document.source);
    assert(!source_length(fixture.scope->document.source) && fixture.observed.valid);
    assert(source_copy_configuration_initialize(&fixture.arena, &edits, &copy, &fixture.map, &fixture.observed,
        &fixture.final, fixture.scope, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    assert(!edits.first); source_text_edits_destroy(&edits); finish_permission_additions(&fixture);
}
