/**
 * @file
 * @brief Compare exact selector matching with independent expansion enumeration.
 * Contract: [configuration](../../../../../docs/implementation/project-configuration.md#t263-selector-matching-contract).
 */
#include <stdio.h>
#include "../common.c"
#include "source/configuration/selectors/selector.h"

static SourceArena patterns;
static SourceIssue issue;

/** Own compiled patterns separately from borrowed SourceText fixture storage. */
static void begin(void) {
    setup_test();
    source_arena_init(&patterns, sources.allocator);
}

/** Compile a real original expression with no alias scope. */
static const DependencySelector *compile(const char *text) {
    const DependencySelector *selector = NULL;
    assert(dependency_selector_compile(&patterns, configuration_test_text(text), NULL,
        &selector, &issue) == DEPENDENCY_SELECTOR_OK);
    return selector;
}

/** Match actual bounded identity metadata and independently account for scratch retirement. */
static bool matches(const DependencySelector *selector, const char *identity) {
    size_t live = live_allocations;
    bool result = false;
    assert(dependency_selector_match(selector, (const unsigned char *)identity, strlen(identity),
        sources.allocator, &result, &issue) == DEPENDENCY_SELECTOR_OK);
    assert(live_allocations == live);
    return result;
}

/** Release compiled storage before original source bytes and require counted balance. */
static void finish(void) { source_arena_destroy(&patterns); destroy_test(); }

#include "support/models.c"
#include "support/scopes.c"
#include "support/ownership.c"

/** Run finite models, actual alias composition and refusal partitions explicitly. */
void run(void) {
    it_matches_every_bounded_scalar_expansion_model();
    it_matches_every_bounded_component_partition_model();
    it_filters_namespace_version_and_unicode_identities_exactly();
    it_copies_package_alias_components_without_cancelling_pattern_traversal();
    it_preserves_outputs_and_cleans_all_selector_allocation_refusals();
    it_matches_deep_and_long_identities_without_a_fixed_limit();
}

/** Zero exit requires every assertion and allocation-owner teardown to pass. */
int main(void) { run(); return EXIT_SUCCESS; }
