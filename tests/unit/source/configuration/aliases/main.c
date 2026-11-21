/**
 * @file
 * @brief Exercise exact two-level scopes, source ownership and preparation refusal.
 * The [configuration contract](../../../../../docs/implementation/project-configuration.md)
 * separates local declarations from package-only target expansion.
 */
#include "../common.c"
#include "source/configuration/aliases/aliases.h"

static SourceArena arena;
static ConfigurationDocument package_document, local_document;
static const ConfigurationAliasScope *package;
static SourceIssue issue;

/** Parse real profile bytes before scope construction; source remains separately owned. */
static void document(ConfigurationDocument *owner, const char *text, ConfigurationProfile profile) {
    SourceSpan span = configuration_test_text(text);
    configuration_document_init(owner, sources.allocator);
    assert(configuration_document_parse(owner, span.source, profile) == CONFIGURATION_OK);
}

/** Prepare the package fixture with a fixed canonical version independent of consumer paths. */
static void begin(const char *aliases) {
    setup_test();
    source_arena_init(&arena, sources.allocator);
    document(&package_document, aliases, CONFIGURATION_PACKAGE);
    const char *version = "/acme/pkg/v1.0.0";
    assert(configuration_package_aliases(&arena, &package_document,
        (const unsigned char *)version, strlen(version), &package, &issue) == CONFIGURATION_SCOPE_OK);
}

/** Use an exact logical module identity, with no physical catalog or inferred parent scope. */
static ConfigurationScopeStatus module(const ConfigurationDocument *doc,
    const ConfigurationAliasScope *parent, const char *name, const ConfigurationAliasScope **output) {
    return configuration_module_aliases(&arena, doc, parent,
        (const unsigned char *)name, strlen(name), output, &issue);
}

/** Compare canonical output with an independently specified complete string. */
static void target(const ConfigurationAliasScope *scope, const char *key, const char *expected) {
    const ConfigurationAliasEntry *entry = configuration_alias_find(scope, configuration_test_text(key));
    assert(entry != NULL);
    assert(strcmp(entry->target->text, expected) == 0);
}

/** Retire scope and parsed nodes before their borrowed source owners. */
static void finish(void) {
    source_arena_destroy(&arena);
    configuration_document_destroy(&package_document);
    destroy_test();
}

#include "support/scopes.c"
#include "support/ownership.c"

/** Run independent scope and resource partitions explicitly. */
void run(void) {
    it_preserves_package_bases_local_isolation_and_original_ownership();
    it_rejects_shadowing_chains_and_foreign_version_scopes();
    it_cleans_all_module_scope_allocation_refusals();
    it_composes_source_operands_with_exact_sorted_scope_lookup();
    it_cleans_all_package_scope_allocation_refusals_and_rejects_invalid_profiles();
    it_preserves_cancelled_targets_and_standard_scope_independence();
}

/** Successful exit means all assertions and counted teardown passed. */
int main(void) { run(); return EXIT_SUCCESS; }
