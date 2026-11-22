/** @file
 * @brief Derive finite selected-root images from actual old/new environments and ready clone maps.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/configuration/prepare.h"
#include "../../../../../../tools/project/source/copy/selectors/reexpress.h"
/** Resolve an exact canonical identity only within an actual prepared view. */
static const DependencyViewEntry *permission_entry(const DependencyView *view, const char *name) {
    for (size_t i = 0; i < view->count; i++) { if (!strcmp(view->entries[i].identity->text, name)) { return &view->entries[i]; } }
    assert(false); return NULL;
}
/** Actual reparse/matching must agree with the complete requested root set;
 * normal structural/test coverage is still computed by the shared evaluator. */
static void check_permission_text(SourceArena *arena, const SourcePackageScope *scope,
    const DependencyView *view, const SourceSelectionRoots *roots) {
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, scope->document.source);
    assert(source_copy_reexpress_selectors(&edits, &scope->document, view, roots->entries, roots->count) == SOURCE_EDITS_OK);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "prepared permissions.ini", &result) == SOURCE_EDITS_OK);
    ConfigurationDocument parsed; configuration_document_init(&parsed, sources.allocator);
    assert(configuration_document_parse(&parsed, result, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    const ConfigurationAliasScope *aliases = NULL; SourceIssue issue;
    assert(configuration_package_aliases(arena, &parsed, (const unsigned char *)scope->version->logical_path,
        strlen(scope->version->logical_path), &aliases, &issue) == CONFIGURATION_SCOPE_OK);
    const DependencySelector *patterns[4]; size_t count = 0;
    for (const ConfigurationSelector *selector = parsed.selectors; selector; selector = selector->next) {
        assert(count < 4);
        assert(dependency_selector_compile(arena, selector->pattern, aliases, &patterns[count++], &issue) == DEPENDENCY_SELECTOR_OK);
    }
    const DependencySelection *selection = NULL;
    assert(dependency_selection_build(arena, view, aliases->base, patterns, count, &selection, &issue) == DEPENDENCY_SELECTION_OK);
    assert(selection->count == roots->count);
    for (const DependencySelectionRoot *root = selection->roots; root; root = root->next) {
        bool found = false;
        for (size_t i = 0; i < roots->count; i++) { if (roots->entries[i] == root->entry) { found = true; } }
        assert(found);
    }
    configuration_document_destroy(&parsed); source_text_edits_destroy(&edits);
}
/** An unchanged wildcard must not acquire a new explicit root; copied
 * configurations instead map exactly the selected roots covered by C. */
static void it_derives_actual_original_and_copied_selection_roots_without_new_wildcard_matches(void) {
    prepare_environment_catalog(); select_main();
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
    SourcePackageEnvironment original; source_package_environment_init(&original, sources.allocator);
    assert(source_package_environment_build(&original, &capture.catalog) == SOURCE_ENVIRONMENT_OK);
    file("generated/project/packages/vendor/pkg/v1.0.0/src/copied", "@!\nother&\n./other\n");
    directory("generated/project/packages/vendor/pkg/v1.0.0/tests/copied");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/copied/check", "@!\n");
    SourceRootCapture final_capture; source_root_capture_init(&final_capture, sources.allocator);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT}, {.path = "generated/standard", .kind = MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&final_capture, roots, 2) == SOURCE_CAPTURE_OK);
    SourcePackageEnvironment final; source_package_environment_init(&final, sources.allocator);
    assert(source_package_environment_build(&final, &final_capture.catalog) == SOURCE_ENVIRONMENT_OK);
    const SourcePackageScope *before = environment_scope(&original, "/vendor/pkg/v1.0.0");
    const SourcePackageScope *after = environment_scope(&final, "/vendor/pkg/v1.0.0");
    assert(before->selection->count == 4 && after->selection->count == 5);
    SourceCloneMap map; source_clone_map_init(&map, sources.allocator, &capture);
    const char *const names[] = {main_identity, "/vendor/pkg/v1.0.0/tests/main/check"};
    const char *const targets[] = {"/vendor/pkg/v1.0.0/src/copied", "/vendor/pkg/v1.0.0/tests/copied/check"};
    for (size_t i = 0; i < 2; i++) {
        const ModuleSource *old = NULL, *now = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, names[i], &old) == MODULE_OK);
        assert(module_catalog_find(&final_capture.catalog, targets[i], &now) == MODULE_OK);
        assert(source_clone_map_add(&map, old, source_path(now->text), now->logical_path, &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator); SourceSelectionRootsIssue issue;
    for (unsigned copied = 0; copied < 2; copied++) {
        SourceSelectionRoots selected = {0};
        assert(source_copy_selected_roots(&arena, before->selection, final.view, copied ? &map : NULL,
            copied != 0, NULL, 0, &selected, &issue) == SOURCE_SELECTION_ROOTS_OK && selected.count == 4);
        bool has_original = false, has_copy = false;
        for (size_t i = 0; i < selected.count; i++) {
            if (!strcmp(selected.entries[i]->identity->text, main_identity)) { has_original = true; }
            if (!strcmp(selected.entries[i]->identity->text, targets[0])) { has_copy = true; }
            if (i) { assert(strcmp(selected.entries[i - 1]->identity->text, selected.entries[i]->identity->text) < 0); }
        }
        assert(has_original == !copied && has_copy == (copied != 0));
        check_permission_text(&arena, after, final.view, &selected);
        if (!copied) {
            SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, before->document.source);
            SourceSelectionRoots preserved = {0}; SourceConfigurationIssue configuration_issue;
            assert(source_copy_configuration_prepare(&arena, &edits, &copy, &map, &original, before,
                &final, after, false, NULL, 0, &preserved, &configuration_issue) == SOURCE_CONFIGURATION_OK);
            assert(preserved.count == selected.count);
            for (size_t i = 0; i < selected.count; i++) { assert(preserved.entries[i] == selected.entries[i]); }
            const SourceText *prepared = NULL;
            assert(source_text_edits_apply(&edits, &sources, "shared package.ini", &prepared) == SOURCE_EDITS_OK);
            const char *prefix = "[aliases]\n~code = ./src\n[dependencies]\n";
            assert(source_length(prepared) >= strlen(prefix) && !memcmp(source_bytes(prepared), prefix, strlen(prefix)));
            check_permission_text(&arena, after, final.view, &preserved);
            source_text_edits_destroy(&edits);
        }
    }
    const SourceText *version_text = NULL; const char *version = "/vendor/pkg/v1.0.0";
    assert(source_manager_add(&sources, "version selector", (const unsigned char *)version, strlen(version), &version_text) == SOURCE_OK);
    const DependencySelector *pattern = NULL; SourceIssue normal_issue;
    assert(dependency_selector_compile(&arena, (SourceSpan){version_text, 0, strlen(version)}, NULL, &pattern, &normal_issue) == DEPENDENCY_SELECTOR_OK);
    const DependencySelection *whole = NULL;
    assert(dependency_selection_build(&arena, original.view, before->aliases->base, &pattern, 1, &whole, &normal_issue) == DEPENDENCY_SELECTION_OK && whole->count == 1);
    SourceSelectionVersionPair pairs[] = {{whole->roots->entry, permission_entry(final.view, "/vendor/pkg/v2.0.0")}, {whole->roots->entry, permission_entry(final.view, "/vendor/pkg/v2.0.0")}};
    SourceSelectionRoots selected = {0};
    assert(source_copy_selected_roots(&arena, whole, final.view, &map, true, pairs, 2, &selected, &issue) == SOURCE_SELECTION_ROOTS_OK);
    assert(selected.count == 1 && selected.entries[0] == pairs[0].final);
    check_permission_text(&arena, after, final.view, &selected);
    SourceSelectionRoots sentinel = selected; pairs[1].final = permission_entry(final.view, version);
    assert(source_copy_selected_roots(&arena, whole, final.view, &map, true, pairs, 2, &selected, &issue) == SOURCE_SELECTION_ROOTS_CONFLICT);
    assert(selected.entries == sentinel.entries && selected.count == sentinel.count && issue.first == &pairs[0] && issue.second == &pairs[1]);
    DependencyViewEntry foreign = *pairs[0].final; pairs[0].final = &foreign;
    assert(source_copy_selected_roots(&arena, whole, final.view, &map, true, pairs, 1, &selected, &issue) == SOURCE_SELECTION_ROOTS_INVALID);
    pairs[0].final = permission_entry(final.view, targets[0]);
    assert(source_copy_selected_roots(&arena, whole, final.view, &map, true, pairs, 1, &selected, &issue) == SOURCE_SELECTION_ROOTS_INVALID);
    const DependencyView *empty = NULL;
    assert(dependency_view_build(&arena, NULL, 0, &empty, &normal_issue) == DEPENDENCY_SELECTION_OK);
    size_t calls = allocation_count;
    assert(source_copy_selected_roots(&arena, whole, empty, NULL, false, NULL, 0, &selected, &issue) == SOURCE_SELECTION_ROOTS_OK);
    assert(!selected.count && !selected.entries && calls == allocation_count);
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena attempt; source_arena_init(&attempt, sources.allocator); selected = sentinel;
        size_t start = allocation_count; if (failure) { failure_at = start + failure; }
        SourceSelectionRootsStatus status = source_copy_selected_roots(&attempt, before->selection, final.view, &map, true, NULL, 0, &selected, &issue);
        if (!failure) { assert(status == SOURCE_SELECTION_ROOTS_OK); total = allocation_count - start; assert(total); }
        else { assert(status == SOURCE_SELECTION_ROOTS_MEMORY && selected.entries == sentinel.entries && selected.count == sentinel.count); }
        failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == baseline);
    }
    assert(source_copy_selected_roots(&arena, whole, final.view, NULL, true, NULL, 0, &selected, &issue) == SOURCE_SELECTION_ROOTS_INVALID);
    assert(source_copy_selected_roots(&arena, whole, final.view, &map, false, NULL, 0, &selected, &issue) == SOURCE_SELECTION_ROOTS_INVALID);
    source_arena_destroy(&arena); source_clone_map_destroy(&map);
    source_package_environment_destroy(&final); source_package_environment_destroy(&original); source_root_capture_destroy(&final_capture);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/copied") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/copied/check") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/copied") == 0); finish_copy();
}
