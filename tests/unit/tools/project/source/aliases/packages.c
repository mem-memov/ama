/** @file
 * @brief Check explicitly copied package alias targets and independent selector edit composition.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/selectors/reexpress.h"
/** Package aliases keep original canonical bindings after the defining version
 * changes, while unrelated repository/selector/comment bytes stay exact. */
static void it_rebases_owned_package_alias_targets_in_independent_destination_scopes(void) {
    const char *const inputs[] = {
        "; package\r\n[aliases]\r\n~things = ./src/data\r\n~runtime = //plain\r\n[dependencies]\r\n~things/*\r\n[repository /vendor/tools]\r\nurl = https://example.invalid/repo\r\ndirectory = ama\r\nrevision = stable",
        "", "[aliases]\n~things = /vendor/pkg/v1.0.0/src/data\n"
    };
    const char *const expected[] = {
        "; package\r\n[aliases]\r\n~things = /vendor/pkg/v1.0.0/src/data\r\n~runtime = //plain\r\n[dependencies]\r\n~things/*\r\n[repository /vendor/tools]\r\nurl = https://example.invalid/repo\r\ndirectory = ama\r\nrevision = stable",
        "", "[aliases]\n~things = /vendor/pkg/v1.0.0/src/data\n"
    };
    for (size_t i = 0; i < sizeof(inputs) / sizeof(*inputs); i++) {
        setup_test(); SourceArena metadata; source_arena_init(&metadata, sources.allocator);
        const SourceText *text = NULL; assert(source_manager_add(&sources, "owned package.ini", (const unsigned char *)inputs[i], strlen(inputs[i]), &text) == SOURCE_OK);
        ConfigurationDocument document; configuration_document_init(&document, sources.allocator);
        assert(configuration_document_parse(&document, text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
        const ConfigurationAliasScope *scope = NULL; SourceIssue issue;
        const char *old = "/vendor/pkg/v1.0.0", *now = "/other/pkg/v2.0.0";
        assert(configuration_package_aliases(&metadata, &document, (const unsigned char *)old, strlen(old), &scope, &issue) == CONFIGURATION_SCOPE_OK);
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, text); SourceCopyAliasesIssue alias_issue;
        assert(source_copy_rebase_aliases(&edits, &document, scope, &alias_issue) == SOURCE_COPY_ALIASES_OK);
        size_t repeat = allocation_count;
        assert(source_copy_rebase_aliases(&edits, &document, scope, &alias_issue) == SOURCE_COPY_ALIASES_OK && allocation_count == repeat);
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edits, &sources, "copied package.ini", &result) == SOURCE_EDITS_OK);
        assert(source_length(result) == strlen(expected[i]) && !memcmp(source_bytes(result), expected[i], strlen(expected[i])));
        ConfigurationDocument final; configuration_document_init(&final, sources.allocator);
        assert(configuration_document_parse(&final, result, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
        const ConfigurationAliasScope *destination = NULL;
        assert(configuration_package_aliases(&metadata, &final, (const unsigned char *)now, strlen(now), &destination, &issue) == CONFIGURATION_SCOPE_OK);
        assert(destination->count == scope->count);
        for (size_t j = 0; j < scope->count; j++) {
            const ConfigurationAliasEntry *entry = configuration_alias_find(destination, scope->entries[j].name);
            assert(entry && !entry->local && !strcmp(entry->target->text, scope->entries[j].target->text));
        }
        configuration_document_destroy(&final); source_text_edits_destroy(&edits);
        configuration_document_destroy(&document); source_arena_destroy(&metadata); destroy_test();
    }
}
/** Canonical alias-target and concrete selector edits share original spans
 * without overlap and remain valid after both independent final resolutions. */
static void it_composes_owned_package_alias_rebasing_with_exact_selector_reexpression_and_failures(void) {
    setup_test(); SourceArena metadata; source_arena_init(&metadata, sources.allocator);
    const char *bytes = "; retain\n[aliases]\n~things = ./src/data\n~runtime = //plain\n[dependencies]\n~things/*\n";
    const SourceText *text = NULL; assert(source_manager_add(&sources, "package.ini", (const unsigned char *)bytes, strlen(bytes), &text) == SOURCE_OK);
    ConfigurationDocument document; configuration_document_init(&document, sources.allocator);
    assert(configuration_document_parse(&document, text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    const ConfigurationAliasScope *scope = NULL; SourceIssue issue; const char *old = "/vendor/pkg/v1.0.0", *now = "/other/pkg/v2.0.0";
    assert(configuration_package_aliases(&metadata, &document, (const unsigned char *)old, strlen(old), &scope, &issue) == CONFIGURATION_SCOPE_OK);
    const char *const names[] = {now, "/vendor/tools/v3.0.0", "/vendor/tools/v3.0.0/src/tool"};
    DependencyViewInput inputs[3];
    for (size_t i = 0; i < 3; i++) {
        const ModulePathValue *value = NULL;
        assert(module_path_from_canonical(&metadata, (const unsigned char *)names[i], strlen(names[i]), &value, &issue) == MODULE_PATH_OK);
        inputs[i] = (DependencyViewInput){value, 0};
    }
    const DependencyView *view = NULL;
    assert(dependency_view_build(&metadata, inputs, 3, &view, &issue) == DEPENDENCY_SELECTION_OK);
    const DependencyViewEntry *root = NULL;
    for (size_t i = 0; i < view->count; i++) { if (view->entries[i].shape.extent == MODULE_PATH_MODULE) { root = &view->entries[i]; } }
    assert(root); SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, text); SourceCopyAliasesIssue alias_issue;
    assert(source_copy_rebase_aliases(&edits, &document, scope, &alias_issue) == SOURCE_COPY_ALIASES_OK);
    assert(source_copy_reexpress_selectors(&edits, &document, view, &root, 1) == SOURCE_EDITS_OK);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "final package.ini", &result) == SOURCE_EDITS_OK);
    ConfigurationDocument final; configuration_document_init(&final, sources.allocator);
    assert(configuration_document_parse(&final, result, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    const ConfigurationAliasScope *destination = NULL;
    assert(configuration_package_aliases(&metadata, &final, (const unsigned char *)now, strlen(now), &destination, &issue) == CONFIGURATION_SCOPE_OK);
    const DependencySelector *pattern = NULL;
    assert(final.selectors && !final.selectors->next);
    assert(dependency_selector_compile(&metadata, final.selectors->pattern, destination, &pattern, &issue) == DEPENDENCY_SELECTOR_OK);
    const DependencySelection *selection = NULL;
    assert(dependency_selection_build(&metadata, view, destination->base, &pattern, 1, &selection, &issue) == DEPENDENCY_SELECTION_OK);
    assert(selection->count == 1 && selection->roots->entry == root);
    configuration_document_destroy(&final); source_text_edits_destroy(&edits);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_text_edits_init(&edits, sources.allocator, text);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyAliasesStatus status = source_copy_rebase_aliases(&edits, &document, scope, &alias_issue);
        if (!failure) { assert(status == SOURCE_COPY_ALIASES_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_COPY_ALIASES_EDITS && edits.status == SOURCE_EDITS_MEMORY && alias_issue.alias && alias_issue.entry); }
        failure_at = 0; source_text_edits_destroy(&edits); assert(live_allocations == baseline);
    }
    ConfigurationAliasEntry wrong_entries[2] = {scope->entries[0], scope->entries[1]}; wrong_entries[0].local = true;
    ConfigurationAliasScope wrong = *scope; wrong.entries = wrong_entries;
    source_text_edits_init(&edits, sources.allocator, text);
    assert(source_copy_rebase_aliases(&edits, &document, &wrong, &alias_issue) == SOURCE_COPY_ALIASES_SCOPE && edits.status != SOURCE_EDITS_OK);
    source_text_edits_destroy(&edits); configuration_document_destroy(&document); source_arena_destroy(&metadata); destroy_test();
}
