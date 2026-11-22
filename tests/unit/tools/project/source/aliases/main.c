/** @file
 * @brief Verify original-scope local alias rebasing without changing keys or unrelated bytes.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/copy/aliases/rebase.h"
static SourceArena arena;
static ConfigurationDocument package_document, local_document;
static const ConfigurationAliasScope *package, *local;
/** Real parsers/resolvers provide authoritative source spans and original bases. */
static void prepare_aliases(const char *text, bool standard) {
    setup_test(); source_arena_init(&arena, sources.allocator);
    const SourceText *package_text = NULL, *module_text = NULL;
    const char *package_bytes = "[aliases]\n~shared = ./src/values\n";
    assert(source_manager_add(&sources, "package.ini", (const unsigned char *)package_bytes, strlen(package_bytes), &package_text) == SOURCE_OK);
    assert(source_manager_add(&sources, "module.ini", (const unsigned char *)text, strlen(text), &module_text) == SOURCE_OK);
    configuration_document_init(&package_document, sources.allocator); configuration_document_init(&local_document, sources.allocator);
    assert(configuration_document_parse(&package_document, package_text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    assert(configuration_document_parse(&local_document, module_text, CONFIGURATION_MODULE) == CONFIGURATION_OK);
    SourceIssue issue; package = NULL; local = NULL;
    const char *version = "/vendor/pkg/v1.0.0";
    if (!standard) {
        assert(configuration_package_aliases(&arena, &package_document, (const unsigned char *)version, strlen(version), &package, &issue) == CONFIGURATION_SCOPE_OK);
    }
    const char *identity = standard ? "//old/module" : "/vendor/pkg/v1.0.0/src/old/module";
    assert(configuration_module_aliases(&arena, &local_document, package,
        (const unsigned char *)identity, strlen(identity), &local, &issue) == CONFIGURATION_SCOPE_OK);
}
/** Retire resolved scopes/documents before their original SourceManager. */
static void finish_aliases(void) {
    source_arena_destroy(&arena); configuration_document_destroy(&local_document);
    configuration_document_destroy(&package_document); destroy_test();
}
/** Reparse copied bytes in an independent destination package with no original
 * shared aliases. Canonical targets must retain the same complete binding.
 */
static void check_destination_scope(const SourceText *text, bool standard, bool conflict) {
    const SourceText *package_text = NULL;
    const char *bytes = conflict ? "[aliases]\n~near = ./src/unrelated\n" : "";
    assert(source_manager_add(&sources, "destination package.ini", (const unsigned char *)bytes, strlen(bytes), &package_text) == SOURCE_OK);
    ConfigurationDocument destination_package, destination_module;
    configuration_document_init(&destination_package, sources.allocator);
    configuration_document_init(&destination_module, sources.allocator);
    assert(configuration_document_parse(&destination_package, package_text, CONFIGURATION_PACKAGE) == CONFIGURATION_OK);
    assert(configuration_document_parse(&destination_module, text, CONFIGURATION_MODULE) == CONFIGURATION_OK);
    const ConfigurationAliasScope *parent = NULL, *result = NULL;
    const char *version = "/destination/pkg/v3.0.0";
    SourceIssue issue;
    if (!standard) {
        assert(configuration_package_aliases(&arena, &destination_package, (const unsigned char *)version,
            strlen(version), &parent, &issue) == CONFIGURATION_SCOPE_OK);
    }
    const char *module = standard ? "//new/path/copied" : "/destination/pkg/v3.0.0/src/new/path/copied";
    ConfigurationScopeStatus status = configuration_module_aliases(&arena, &destination_module, parent,
        (const unsigned char *)module, strlen(module), &result, &issue);
    if (conflict) { assert(status == CONFIGURATION_SCOPE_INVALID && !result); }
    else {
        assert(status == CONFIGURATION_SCOPE_OK && result->count == local->count);
        for (size_t i = 0; i < local->count; i++) {
            const ConfigurationAliasEntry *copied = configuration_alias_find(result, local->entries[i].name);
            assert(copied && copied->local && !strcmp(copied->target->text, local->entries[i].target->text));
        }
    }
    configuration_document_destroy(&destination_module);
    configuration_document_destroy(&destination_package);
}
/** Package alias expansion, relative targets, roots and multiword keys use exact
 * canonical bindings; CRLF/comments/spacing remain byte-identical outside targets. */
static void it_rebases_actual_local_alias_targets_and_preserves_all_other_ini_bytes(void) {
    const char *const inputs[] = {"; kept\r\n~long alias = ~shared/item\r\n~near = ../other\r\n~std = //text\r\n",
        "~near = ../other\n~root = //text\n", "", "~root = //text\n"};
    const char *const expected[] = {"; kept\r\n~long alias = /vendor/pkg/v1.0.0/src/values/item\r\n~near = /vendor/pkg/v1.0.0/src/other\r\n~std = //text\r\n",
        "~near = //other\n~root = //text\n", "", "~root = //text\n"};
    for (unsigned profile = 0; profile < 4; profile++) {
        prepare_aliases(inputs[profile], profile == 1);
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, local_document.source);
        SourceCopyAliasesIssue issue;
        assert(source_copy_rebase_aliases(&edits, &local_document, local, &issue) == SOURCE_COPY_ALIASES_OK && !issue.alias);
        size_t before = allocation_count;
        assert(source_copy_rebase_aliases(&edits, &local_document, local, &issue) == SOURCE_COPY_ALIASES_OK && allocation_count == before);
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edits, &sources, "copied.ini", &result) == SOURCE_EDITS_OK);
        assert(source_length(result) == strlen(expected[profile]) && !memcmp(source_bytes(result), expected[profile], strlen(expected[profile])));
        check_destination_scope(result, profile == 1, false);
        assert(source_length(local_document.source) == strlen(inputs[profile]) && !memcmp(source_bytes(local_document.source), inputs[profile], strlen(inputs[profile])));
        assert(source_copy_rebase_aliases(&edits, &local_document, local, &issue) == SOURCE_COPY_ALIASES_INVALID && edits.valid);
        source_text_edits_destroy(&edits); finish_aliases();
    }
}
/** Different text/scope profiles, alias-table mismatch and conflicting pending
 * target edits cannot be accepted as a successfully rebased local environment. */
static void it_refuses_foreign_alias_authorities_and_overlapping_target_edits(void) {
    for (unsigned profile = 0; profile < 8; profile++) {
        prepare_aliases("~near = ../other\n", false);
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, local_document.source);
        ConfigurationAliasScope scope = *local;
        if (profile == 0) { scope.kind = CONFIGURATION_PACKAGE_SCOPE; }
        if (profile == 1) { scope.source = package_document.source; }
        if (profile == 2) { scope.count = 0; }
        if (profile == 3) { scope.entries = NULL; }
        if (profile == 4) { local_document.valid = false; }
        if (profile == 5) { local_document.profile = CONFIGURATION_PACKAGE; }
        if (profile == 6) { edits.original = package_document.source; }
        if (profile == 7) {
            assert(source_text_edits_add(&edits, local_document.aliases->target, (const unsigned char *)"//different", 11) == SOURCE_EDITS_OK);
        }
        SourceCopyAliasesIssue issue;
        SourceCopyAliasesStatus status = source_copy_rebase_aliases(&edits, &local_document, &scope, &issue);
        assert(status == (profile == 2 ? SOURCE_COPY_ALIASES_SCOPE : profile == 7 ? SOURCE_COPY_ALIASES_EDITS : SOURCE_COPY_ALIASES_INVALID));
        assert(edits.status != SOURCE_EDITS_OK);
        const SourceText *result = NULL;
        assert(source_text_edits_apply(&edits, &sources, "refused.ini", &result) != SOURCE_EDITS_OK && !result);
        source_text_edits_destroy(&edits); finish_aliases();
    }
}
/** Rebasing targets never authorizes shadowing an alias in the new package;
 * actual final scope admission must still refuse that unchanged-key collision.
 */
static void it_checks_rebased_aliases_in_the_real_destination_scope_without_old_package_borrowing(void) {
    prepare_aliases("~near = ../other\n", false);
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, local_document.source);
    SourceCopyAliasesIssue issue;
    assert(source_copy_rebase_aliases(&edits, &local_document, local, &issue) == SOURCE_COPY_ALIASES_OK);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "copied.ini", &result) == SOURCE_EDITS_OK);
    check_destination_scope(result, false, true);
    source_text_edits_destroy(&edits); finish_aliases();
}
/** Fault every actual edit allocation; borrowed parser/scope owners remain healthy
 * and each failed owned prefix retires back to the same baseline. */
static void it_retires_every_rebased_alias_edit_allocation_and_rejects_invalid_handoffs(void) {
    prepare_aliases("~a = ../a\n~b = ~shared/b\n", false);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, local_document.source);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyAliasesIssue issue;
        SourceCopyAliasesStatus status = source_copy_rebase_aliases(&edits, &local_document, local, &issue);
        if (!failure) { assert(status == SOURCE_COPY_ALIASES_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_COPY_ALIASES_EDITS && edits.status == SOURCE_EDITS_MEMORY && issue.alias && issue.entry); }
        failure_at = 0; source_text_edits_destroy(&edits); assert(live_allocations == baseline);
    }
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, local_document.source); SourceCopyAliasesIssue issue;
    assert(source_copy_rebase_aliases(NULL, &local_document, local, &issue) == SOURCE_COPY_ALIASES_INVALID);
    assert(source_copy_rebase_aliases(&edits, &local_document, local, NULL) == SOURCE_COPY_ALIASES_INVALID && edits.status == SOURCE_EDITS_OK);
    assert(source_copy_rebase_aliases(&edits, NULL, local, &issue) == SOURCE_COPY_ALIASES_INVALID);
    assert(source_copy_rebase_aliases(&edits, &local_document, local, &issue) == SOURCE_COPY_ALIASES_EDITS);
    source_text_edits_destroy(&edits); finish_aliases();
}
/** Explicit registration keeps all authority, byte and resource partitions live. */
void run(void) {
    it_checks_rebased_aliases_in_the_real_destination_scope_without_old_package_borrowing();
    it_rebases_actual_local_alias_targets_and_preserves_all_other_ini_bytes();
    it_refuses_foreign_alias_authorities_and_overlapping_target_edits();
    it_retires_every_rebased_alias_edit_allocation_and_rejects_invalid_handoffs();
}
/** All parsed/resolved/edit owners retire before counted source teardown. */
int main(void) { run(); return 0; }
