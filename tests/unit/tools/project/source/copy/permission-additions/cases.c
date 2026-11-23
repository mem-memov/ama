/** @file
 * @brief Verify concrete extension policy through independent final compiler and binding/site proof.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../../tools/project/source/copy/configuration/prepare.h"
#include "../../../../../../../tools/project/source/copy/configuration/check.h"
/** Re-express the exact preserved-plus-added union, then let the real compiler
 * decide all actual final scopes and compare every original/copied reference site. */
static void verify_permission_addition(PermissionAdditionFixture *fixture, const SourceSelectionRoots *added) {
    const SourcePackageScope *before = fixture->new_version ? fixture->scope
        : environment_scope(&fixture->original, fixture->scope->version->logical_path);
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, before->document.source);
    SourceSelectionRoots requested = {0}; SourceConfigurationIssue configuration_issue;
    SourceConfigurationStatus status = fixture->new_version
        ? source_copy_configuration_initialize(&fixture->arena, &edits, &copy, &fixture->map,
            &fixture->observed, &fixture->final, fixture->scope, &requested, &configuration_issue)
        : source_copy_configuration_prepare(&fixture->arena, &edits, &copy, &fixture->map,
            &fixture->original, before, &fixture->final, fixture->scope, false, NULL, 0,
            &requested, &configuration_issue);
    assert(status == SOURCE_CONFIGURATION_OK);
    assert(requested.count == fixture->baseline.count + added->count);
    for (size_t i = 0; i < requested.count; i++) {
        bool found = false;
        for (size_t j = 0; j < fixture->baseline.count; j++) { if (requested.entries[i] == fixture->baseline.entries[j]) { found = true; } }
        for (size_t j = 0; j < added->count; j++) { if (requested.entries[i] == added->entries[j]) { found = true; } }
        assert(found);
    }
    check_permission_text(&fixture->arena, fixture->scope, fixture->final.view, &requested);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, source_path(fixture->scope->document.source), &result) == SOURCE_EDITS_OK);
    permission_source_file(source_path(result), result);
    SourceRootCapture verified; source_root_capture_init(&verified, sources.allocator);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT}, {.path = "generated/standard", .kind = MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&verified, roots, 2) == SOURCE_CAPTURE_OK);
    FILE *stream = fopen("generated/addition-request.json", "wb"); assert(stream);
    assert(delivery_source_snapshot_write(stream, &verified.view) && fclose(stream) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/addition-request.json", &verified.view, NULL) == SOURCE_COMPILER_OK);
    SourceSitesIssue issue;
    assert(source_graph_sites(analysis.original.response.definitions, client.response.definitions, &fixture->map, NULL, 0, (SourceRegistryRetirement){0}, fixture->provenance, &issue) == SOURCE_SITES_OK);
    SourcePackageEnvironment reparsed; source_package_environment_init(&reparsed, sources.allocator);
    assert(source_package_environment_build(&reparsed, &verified.catalog) == SOURCE_ENVIRONMENT_OK);
    const SourcePackageScope *actual = environment_scope(&reparsed, fixture->scope->version->logical_path);
    SourceConfigurationCheckIssue checked;
    size_t check_allocations = allocation_count; failure_at = check_allocations + 1;
    assert(source_copy_configuration_check(&fixture->final, fixture->scope, &requested, &reparsed, actual, &checked) == SOURCE_CONFIGURATION_CHECK_OK);
    assert(!checked.expected && !checked.actual && allocation_count == check_allocations); failure_at = 0;
    source_package_environment_destroy(&reparsed);
    source_compiler_client_destroy(&client); source_root_capture_destroy(&verified); source_text_edits_destroy(&edits);
}
/** Duplicate actual external edges produce one concrete addition; existing
 * permissions, same-version cloned targets and standard edges produce none. */
static void it_derives_only_authorized_concrete_permission_additions_and_passes_final_compiler_sites(void) {
    const SourceCopyDependencyMode modes[] = {SOURCE_COPY_EXTEND_PACKAGE, SOURCE_COPY_EXTEND_PACKAGE, SOURCE_COPY_CLONE_DEPENDENCIES, SOURCE_COPY_EXTEND_PACKAGE};
    for (size_t i = 0; i < 4; i++) {
        PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, modes[i], i == 1, 0, i == 3, false);
        SourcePermissionIssue issue; SourceSelectionRoots added = {0};
        assert(source_copy_permission_additions(&fixture.arena, &copy, &fixture.map, &fixture.final,
            fixture.scope, &fixture.baseline, &added, &issue) == SOURCE_PERMISSION_OK);
        assert(added.count == (i == 0 || i == 3 ? 1 : 0));
        if (added.count) { assert(!strcmp(added.entries[0]->identity->text, "/vendor/pkg/v1.0.0/src/other")); }
        assert(!issue.source && !issue.edge && !issue.expected);
        verify_permission_addition(&fixture, &added); finish_permission_additions(&fixture);
    }
}
/** Refused default-mode edges or missing actual ordinary/standard targets cannot
 * publish even a previously collected extension prefix. */
static void it_refuses_uncovered_or_missing_copy_permission_targets_before_publishing_additions(void) {
    for (unsigned profile = 0; profile < 3; profile++) {
        PermissionAdditionFixture fixture;
        prepare_permission_additions(&fixture, profile ? SOURCE_COPY_EXTEND_PACKAGE : SOURCE_COPY_KEEP_EXTERNAL, false, profile, false, false);
        SourceSelectionRoots sentinel = fixture.baseline, added = sentinel; SourcePermissionIssue issue;
        SourcePermissionStatus status = source_copy_permission_additions(&fixture.arena, &copy, &fixture.map,
            &fixture.final, fixture.scope, &fixture.baseline, &added, &issue);
        assert(status == (profile ? SOURCE_PERMISSION_MISSING : SOURCE_PERMISSION_UNDECLARED));
        assert(added.entries == sentinel.entries && added.count == sentinel.count && issue.source && issue.edge);
        assert(!strcmp(issue.expected, profile == 2 ? "//plain" : "/vendor/pkg/v1.0.0/src/other"));
        finish_permission_additions(&fixture);
    }
}
/** All scratch/output allocations remain owned; foreign scopes/roots and
 * unsealed correspondence cannot become permission authority. */
static void it_retires_permission_addition_failures_and_refuses_foreign_baselines(void) {
    for (unsigned covered = 0; covered < 2; covered++) {
        PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, covered != 0, 0, false, false);
        size_t total = 0, baseline = live_allocations;
        for (size_t failure = 0; !failure || failure <= total; failure++) {
            SourceArena attempt; source_arena_init(&attempt, sources.allocator);
            SourceSelectionRoots output = fixture.baseline; SourcePermissionIssue issue;
            size_t start = allocation_count; if (failure) { failure_at = start + failure; }
            SourcePermissionStatus status = source_copy_permission_additions(&attempt, &copy, &fixture.map, &fixture.final,
                fixture.scope, &fixture.baseline, &output, &issue);
            if (!failure) { assert(status == SOURCE_PERMISSION_OK); total = allocation_count - start; assert(total); }
            else { assert(status == SOURCE_PERMISSION_MEMORY && output.entries == fixture.baseline.entries && output.count == fixture.baseline.count); }
            failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == baseline);
        }
        SourcePackageScope foreign = *fixture.scope; SourcePermissionIssue issue; SourceSelectionRoots output = {0};
        assert(source_copy_permission_additions(&fixture.arena, &copy, &fixture.map, &fixture.final,
            &foreign, &fixture.baseline, &output, &issue) == SOURCE_PERMISSION_INVALID);
        DependencyViewEntry fake = fixture.final.view->entries[0]; const DependencyViewEntry *entries[] = {&fake};
        SourceSelectionRoots invalid = {entries, 1};
        assert(source_copy_permission_additions(&fixture.arena, &copy, &fixture.map, &fixture.final,
            fixture.scope, &invalid, &output, &issue) == SOURCE_PERMISSION_INVALID);
        SourceCloneMap open; source_clone_map_init(&open, sources.allocator, &capture);
        assert(source_copy_permission_additions(&fixture.arena, &copy, &open, &fixture.final,
            fixture.scope, &fixture.baseline, &output, &issue) == SOURCE_PERMISSION_INVALID);
        source_clone_map_destroy(&open); finish_permission_additions(&fixture);
    }
}

/** Preparation failure never publishes an exact-set result; both scratch and
 * partially prepared edit owners retire at every real allocation ordinal. */
static void it_retires_composed_configuration_failures_and_rejects_foreign_authority(void) {
    PermissionAdditionFixture fixture; prepare_permission_additions(&fixture, SOURCE_COPY_EXTEND_PACKAGE, false, 0, false, false);
    const SourcePackageScope *before = environment_scope(&fixture.original, fixture.scope->version->logical_path);
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, before->document.source);
        SourceSelectionRoots output = fixture.baseline; SourceConfigurationIssue issue;
        size_t start = allocation_count; if (failure) { failure_at = start + failure; }
        SourceConfigurationStatus status = source_copy_configuration_prepare(&arena, &edits, &copy, &fixture.map,
            &fixture.original, before, &fixture.final, fixture.scope, false, NULL, 0, &output, &issue);
        if (!failure) { assert(status == SOURCE_CONFIGURATION_OK && output.count == 1); total = allocation_count - start; assert(total); }
        else {
            assert(status == SOURCE_CONFIGURATION_MEMORY
                || (status == SOURCE_CONFIGURATION_ROOTS && issue.roots_status == SOURCE_SELECTION_ROOTS_MEMORY)
                || (status == SOURCE_CONFIGURATION_PERMISSIONS && issue.permission_status == SOURCE_PERMISSION_MEMORY)
                || (status == SOURCE_CONFIGURATION_EDITS && issue.edits_status == SOURCE_EDITS_MEMORY));
            assert(output.entries == fixture.baseline.entries && output.count == fixture.baseline.count);
        }
        failure_at = 0; source_text_edits_destroy(&edits); source_arena_destroy(&arena); assert(live_allocations == baseline);
    }
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, before->document.source);
    SourceSelectionRoots output = fixture.baseline; SourceConfigurationIssue issue;
    SourcePackageScope foreign = *before;
    assert(source_copy_configuration_prepare(&fixture.arena, &edits, &copy, &fixture.map,
        &fixture.original, &foreign, &fixture.final, fixture.scope, false, NULL, 0, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    assert(!edits.first && output.entries == fixture.baseline.entries);
    SourceSelectionVersionPair pair = {0};
    assert(source_copy_configuration_prepare(&fixture.arena, &edits, &copy, &fixture.map,
        &fixture.original, before, &fixture.final, fixture.scope, false, &pair, 0, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    assert(!edits.first);
    source_text_edits_destroy(&edits);
    const SourcePackageScope *source_scope = environment_scope(&fixture.original, "/vendor/pkg/v1.0.0");
    source_text_edits_init(&edits, sources.allocator, source_scope->document.source);
    assert(source_copy_configuration_prepare(&fixture.arena, &edits, &copy, &fixture.map,
        &fixture.original, source_scope, &fixture.final, fixture.scope, false, NULL, 0, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    assert(!edits.first);
    pair.original = permission_entry(fixture.original.view, "/vendor/pkg/v1.0.0");
    pair.final = permission_entry(fixture.final.view, "/vendor/pkg/v2.0.0");
    assert(source_copy_configuration_prepare(&fixture.arena, &edits, &copy, &fixture.map,
        &fixture.original, source_scope, &fixture.final, fixture.scope, true, &pair, 1, &output, &issue) == SOURCE_CONFIGURATION_OK);
    assert(output.count == 1 && !strcmp(output.entries[0]->identity->text, "/vendor/pkg/v1.0.0/src/other"));
    const SourceText *prepared = NULL;
    assert(source_text_edits_apply(&edits, &sources, "copied package.ini", &prepared) == SOURCE_EDITS_OK);
    const char *comment = "; original\n";
    assert(source_length(prepared) > strlen(comment) && !memcmp(source_bytes(prepared), comment, strlen(comment)));
    assert(source_copy_configuration_prepare(&fixture.arena, &edits, &copy, &fixture.map,
        &fixture.original, source_scope, &fixture.final, fixture.scope, true, &pair, 1, &output, &issue) == SOURCE_CONFIGURATION_INVALID);
    source_text_edits_destroy(&edits);
    source_text_edits_init(&edits, sources.allocator, source_scope->document.source);
    pair.final = permission_entry(fixture.final.view, "/vendor/pkg/v2.0.0/src/copied");
    assert(source_copy_configuration_prepare(&fixture.arena, &edits, &copy, &fixture.map,
        &fixture.original, source_scope, &fixture.final, fixture.scope, true, &pair, 1, &output, &issue) == SOURCE_CONFIGURATION_ROOTS);
    assert(issue.roots_status == SOURCE_SELECTION_ROOTS_INVALID && !edits.first);
    source_text_edits_destroy(&edits); finish_permission_additions(&fixture);
}
