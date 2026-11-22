/**
 * @file
 * @brief Check finite selected-root unions against explicit independent physical-unit relations.
 * Contract: [selection](../../../../../docs/implementation/project-configuration.md#t262-selection-and-explicit-edge-contract).
 */
#include <stdint.h>
#include "../common.c"
#include "source/configuration/selection/selection.h"

static SourceArena metadata, results;
static SourceIssue issue;

/** Copy canonical metadata without fabricating source locations. */
static const ModulePathValue *path(const char *name) {
    const ModulePathValue *value = NULL;
    assert(module_path_from_canonical(&metadata, (const unsigned char *)name, strlen(name), &value, &issue) == MODULE_PATH_OK);
    return value;
}
/** Compile actual original selector bytes without aliases. */
static const DependencySelector *pattern(const char *name) {
    const DependencySelector *value = NULL;
    assert(dependency_selector_compile(&metadata, configuration_test_text(name), NULL, &value, &issue) == DEPENDENCY_SELECTOR_OK);
    return value;
}
/** Initialize separate long-lived metadata and disposable selection owners. */
static void begin(void) {
    setup_test();
    source_arena_init(&metadata, sources.allocator);
    source_arena_init(&results, sources.allocator);
}
/** Retire borrowed view/identity relationships in reverse owner order. */
static void finish(void) {
    source_arena_destroy(&results);
    source_arena_destroy(&metadata);
    destroy_test();
}

#include "support/model.c"
#include "support/edges.c"
#include "support/ownership.c"
#include "support/catalog.c"
#include "support/catalog-view.c"
#include "support/lifetime.c"
#include "support/standard.c"

/** Run independent union, policy, provenance and failure boundaries explicitly. */
void run(void) {
    it_matches_all_bounded_root_unions_and_physical_test_associations();
    it_checks_origin_versions_and_source_test_policy_before_privileges();
    it_validates_view_metadata_and_cleans_every_allocation_refusal();
    it_composes_real_selected_catalog_paths_with_physical_test_coverage();
    it_builds_shared_dependency_views_from_actual_catalogs_with_owned_lifetimes();
    it_refuses_invalid_catalog_views_and_retires_every_preparation_allocation();
    it_preserves_unicode_identity_and_independent_view_pattern_lifetimes();
    it_checks_standard_dependency_partitions_without_package_scope_or_effects();
    it_checks_standard_local_alias_targets_after_full_normalization();
}
/** Require all assertions and counted owners to finish. */
int main(void) { run(); return EXIT_SUCCESS; }
