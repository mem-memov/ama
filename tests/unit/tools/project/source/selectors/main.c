/** @file
 * @brief Verify concrete dependency re-expression with real parsed documents and selection domains.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/copy/selectors/reexpress.h"
static SourceArena arena;
static ConfigurationDocument document;
static const DependencyView *view;
static const ModulePathValue *owner;
#define A "/vendor/pkg/v1.0.0/src/a"
#define B "/vendor/pkg/v1.0.0/src/b"
/** Successful normal forms and a complete dependency view own all root authority. */
static void prepare_selectors(const char *bytes) {
    setup_test(); source_arena_init(&arena, sources.allocator);
    const char *const names[] = {"/here/pkg/v1.0.0", "/vendor/pkg/v1.0.0", A, B,
        "/vendor/pkg/v1.0.0/tests/a/check"};
    DependencyViewInput inputs[5]; SourceIssue issue;
    for (size_t i = 0; i < 5; i++) {
        const ModulePathValue *value = NULL;
        assert(module_path_from_canonical(&arena, (const unsigned char *)names[i], strlen(names[i]), &value, &issue) == MODULE_PATH_OK);
        inputs[i] = (DependencyViewInput){value, i == 4 ? 1 : 0}; if (!i) { owner = value; }
    }
    assert(dependency_view_build(&arena, inputs, 5, &view, &issue) == DEPENDENCY_SELECTION_OK);
    const SourceText *text = NULL;
    assert(source_manager_add(&sources, "package.ini", (const unsigned char *)bytes, strlen(bytes), &text) == SOURCE_OK);
    configuration_document_init(&document, sources.allocator);
    assert(configuration_document_parse(&document, text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
}
/** Retire documents/scopes before source-manager and counted fixture ownership. */
static void finish_selectors(void) {
    configuration_document_destroy(&document); source_arena_destroy(&arena); destroy_test();
}
/** Exact names identify entries only inside the already verified domain. */
static const DependencyViewEntry *entry(const char *name) {
    for (size_t i = 0; i < view->count; i++) { if (!strcmp(view->entries[i].identity->text, name)) { return &view->entries[i]; } }
    assert(false); return NULL;
}
/** Independently parse/compile/select the prepared document, never infer selected
 * roots from replacement text or from the emitting function's implementation. */
static void check_selection(const SourceText *text, size_t count) {
    ConfigurationDocument final; configuration_document_init(&final, sources.allocator);
    assert(configuration_document_parse(&final, text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    const DependencySelector *patterns[2]; size_t used = 0; SourceIssue issue;
    for (const ConfigurationSelector *row = final.selectors; row; row = row->next) {
        assert(used < 2);
        assert(dependency_selector_compile(&arena, row->pattern, NULL, &patterns[used++], &issue) == DEPENDENCY_SELECTOR_OK);
    }
    const DependencySelection *selection = NULL;
    assert(dependency_selection_build(&arena, view, owner, patterns, used, &selection, &issue) == DEPENDENCY_SELECTION_OK);
    assert(selection->count == count && used == count);
    for (const DependencySelectionRoot *root = selection->roots; root; root = root->next) {
        assert(root->entry == entry(A) || root->entry == entry(B));
    }
    assert(dependency_selection_covers(selection, entry(A)->identity) == (count != 0));
    assert(dependency_selection_covers(selection, entry(B)->identity) == (count != 0));
    assert(dependency_selection_covers(selection, entry("/vendor/pkg/v1.0.0/tests/a/check")->identity) == (count != 0));
    configuration_document_destroy(&final);
}
/** Original parsed spans preserve comments, aliases, CRLF and final newline
 * independently of root order/repetition and section placement. */
static void it_reexpresses_concrete_sets_with_independent_final_selection_and_exact_bytes(void) {
    const char *const original[] = {
        "; keep\n[dependencies]\n/vendor/*/v1.0.0/src/*\n; comment\n/vendor/pkg/v1.0.0/tests/a/check\n[aliases]\n~x = /vendor/pkg/v1.0.0/src\n",
        "[dependencies]", "[dependencies]\r\n[aliases]\r\n~x = /vendor/pkg/v1.0.0/src\r\n",
        "", "; untouched", "; untouched\n", "[dependencies]\n" A,
        "[dependencies]\n" A "\n; keep\n" B "\n"
    };
    const char *const expected[] = {
        "; keep\n[dependencies]\n" A "\n" B "\n; comment\n\n[aliases]\n~x = /vendor/pkg/v1.0.0/src\n",
        "[dependencies]\n" A "\n" B, "[dependencies]\r\n" A "\r\n" B "\r\n[aliases]\r\n~x = /vendor/pkg/v1.0.0/src\r\n",
        "[dependencies]\n" A "\n" B "\n", "; untouched\n[dependencies]\n" A "\n" B,
        "; untouched\n[dependencies]\n" A "\n" B "\n", "[dependencies]\n" A "\n" B,
        "[dependencies]\n\n; keep\n\n"
    };
    for (size_t i = 0; i < sizeof(original) / sizeof(*original); i++) {
        prepare_selectors(original[i]); SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, document.source);
        const DependencyViewEntry *roots[] = {entry(B), entry(A), entry(B)}; size_t count = i == 7 ? 0 : 3;
        assert(source_copy_reexpress_selectors(&edits, &document, view, roots, count) == SOURCE_EDITS_OK);
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edits, &sources, "prepared.ini", &result) == SOURCE_EDITS_OK);
        assert(source_length(result) == strlen(expected[i]) && !memcmp(source_bytes(result), expected[i], strlen(expected[i])));
        check_selection(result, count ? 2 : 0);
        assert(source_copy_reexpress_selectors(&edits, &document, view, roots, count) == SOURCE_EDITS_INVALID);
        source_text_edits_destroy(&edits); finish_selectors();
    }
}
/** Invalid authority, overlapping edits and every allocation refusal cannot
 * publish a partial configuration or retain unowned failed scratch. */
static void it_refuses_foreign_selector_authority_conflicts_and_every_allocation_failure(void) {
    prepare_selectors("[dependencies]\n/vendor/*/v1.0.0/src/*\n/vendor/pkg/v1.0.0/tests/a/check\n");
    const DependencyViewEntry *roots[] = {entry(A), entry(B)};
    for (unsigned i = 0; i < 6; i++) {
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, document.source);
        ConfigurationDocument wrong = document; wrong.profile = CONFIGURATION_MODULE;
        DependencyViewEntry foreign = *entry(A); const DependencyViewEntry *bad[] = {&foreign};
        const SourceText *text = NULL;
        assert(source_manager_add(&sources, "same bytes.ini", source_bytes(document.source), source_length(document.source), &text) == SOURCE_OK);
        if (i == 4) { wrong = document; wrong.source = text; }
        if (i == 5) { edits.status = SOURCE_EDITS_CONFLICT; }
        SourceEditsStatus status = source_copy_reexpress_selectors(&edits, i == 0 || i == 4 ? &wrong : &document,
            i == 1 ? NULL : view, i == 2 ? NULL : i == 3 ? bad : roots, i == 3 ? 1 : 2);
        assert(status == (i == 5 ? SOURCE_EDITS_CONFLICT : SOURCE_EDITS_INVALID));
        assert(source_text_edits_apply(&edits, &sources, "refused.ini", &text) != SOURCE_EDITS_OK);
        source_text_edits_destroy(&edits);
    }
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, document.source);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceEditsStatus status = source_copy_reexpress_selectors(&edits, &document, view, roots, 2);
        if (!failure) { assert(status == SOURCE_EDITS_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_EDITS_MEMORY && edits.status == SOURCE_EDITS_MEMORY); }
        failure_at = 0; source_text_edits_destroy(&edits); assert(live_allocations == baseline);
    }
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, document.source);
    SourceSpan conflict = document.selectors->pattern; conflict.start++;
    assert(source_text_edits_add(&edits, conflict, (const unsigned char *)"x", 1) == SOURCE_EDITS_OK);
    assert(source_copy_reexpress_selectors(&edits, &document, view, roots, 2) == SOURCE_EDITS_CONFLICT);
    source_text_edits_destroy(&edits); finish_selectors();
}
/** A new actual module matching an old mask must not become a new selected
 * root silently. Derive requested roots from actual original evaluation, then
 * re-evaluate independently against the augmented final domain. */
static void it_preserves_original_matches_when_the_final_domain_gains_a_module(void) {
    prepare_selectors("[dependencies]\n/vendor/pkg/v1.0.0/src/*\n");
    SourceIssue issue; const DependencySelector *pattern = NULL;
    assert(dependency_selector_compile(&arena, document.selectors->pattern, NULL, &pattern, &issue) == DEPENDENCY_SELECTOR_OK);
    const DependencySelection *original = NULL;
    assert(dependency_selection_build(&arena, view, owner, &pattern, 1, &original, &issue) == DEPENDENCY_SELECTION_OK);
    assert(original->count == 2);
    const ModulePathValue *new_module = NULL;
    const char *name = "/vendor/pkg/v1.0.0/src/new";
    assert(module_path_from_canonical(&arena, (const unsigned char *)name, strlen(name), &new_module, &issue) == MODULE_PATH_OK);
    DependencyViewInput inputs[6];
    for (size_t i = 0; i < 5; i++) { inputs[i] = (DependencyViewInput){view->entries[i].identity, view->entries[i].test_directory_prefix}; }
    inputs[5] = (DependencyViewInput){new_module, 0};
    assert(dependency_view_build(&arena, inputs, 6, &view, &issue) == DEPENDENCY_SELECTION_OK);
    const DependencySelection *broadened = NULL;
    assert(dependency_selection_build(&arena, view, owner, &pattern, 1, &broadened, &issue) == DEPENDENCY_SELECTION_OK);
    assert(broadened->count == 3 && dependency_selection_covers(broadened, new_module));
    const DependencyViewEntry *roots[2]; size_t count = 0;
    for (const DependencySelectionRoot *root = original->roots; root; root = root->next) { roots[count++] = entry(root->entry->identity->text); }
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, document.source);
    assert(source_copy_reexpress_selectors(&edits, &document, view, roots, count) == SOURCE_EDITS_OK);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "finite.ini", &result) == SOURCE_EDITS_OK);
    check_selection(result, 2); source_text_edits_destroy(&edits);
    source_text_edits_init(&edits, sources.allocator, document.source);
    roots[0] = entry("/vendor/pkg/v1.0.0");
    assert(source_copy_reexpress_selectors(&edits, &document, view, roots, 1) == SOURCE_EDITS_OK);
    assert(source_text_edits_apply(&edits, &sources, "version.ini", &result) == SOURCE_EDITS_OK);
    const char *expected = "[dependencies]\n/vendor/pkg/v1.0.0\n";
    assert(source_length(result) == strlen(expected) && !memcmp(source_bytes(result), expected, strlen(expected)));
    ConfigurationDocument final; configuration_document_init(&final, sources.allocator);
    assert(configuration_document_parse(&final, result, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    assert(dependency_selector_compile(&arena, final.selectors->pattern, NULL, &pattern, &issue) == DEPENDENCY_SELECTOR_OK);
    assert(dependency_selection_build(&arena, view, owner, &pattern, 1, &broadened, &issue) == DEPENDENCY_SELECTION_OK);
    assert(broadened->count == 1 && dependency_selection_covers(broadened, new_module));
    configuration_document_destroy(&final); source_text_edits_destroy(&edits); finish_selectors();
    const char *const unchanged[] = {"", "; untouched", "[dependencies]", "[aliases]\r\n~x = /vendor/pkg/v1.0.0/src\r\n"};
    for (size_t i = 0; i < sizeof(unchanged) / sizeof(*unchanged); i++) {
        prepare_selectors(unchanged[i]); source_text_edits_init(&edits, sources.allocator, document.source);
        assert(source_copy_reexpress_selectors(&edits, &document, view, NULL, 0) == SOURCE_EDITS_OK && !edits.first);
        source_text_edits_destroy(&edits); finish_selectors();
    }
}
/** Explicit component and real parser/selection composition acceptance. */
void run(void) {
    it_reexpresses_concrete_sets_with_independent_final_selection_and_exact_bytes();
    it_refuses_foreign_selector_authority_conflicts_and_every_allocation_failure();
    it_preserves_original_matches_when_the_final_domain_gains_a_module();
}
/** Run the concrete-selector parser and exact-selection cases through the leaf harness. */
int main(void) { run(); return 0; }
