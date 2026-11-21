/**
 * @file
 * @brief Verify exact input graph fixed points and real source/native closure capture.
 */
#include "../common.c"
#include "project/inputs/inputs.h"

/** Admit an exact fixture node and require successful checked publication. */
static ProjectInput *put(ProjectInputs *inputs, ProjectInputKind kind, const char *key, const char *bytes) {
    ProjectInput *node = NULL;
    SourceStatus status = project_inputs_put(inputs, kind, key,
        (const unsigned char *)bytes, strlen(bytes), &node);
    assert(status == SOURCE_OK);
    return node;
}
/** Add one checked dependency using the public builder boundary. */
static void depend(ProjectInputs *inputs, ProjectInput *consumer, ProjectInput *input) {
    SourceStatus status = project_inputs_depend(inputs, consumer, input);
    assert(status == SOURCE_OK);
}
/** Observe a compared node through the public query rather than private graph fields. */
static bool changed(const ProjectInputs *inputs, ProjectInputKind kind, const char *key) {
    bool value = false;
    bool found = project_inputs_changed(inputs, kind, key, &value);
    assert(found);
    return value;
}
#include "support/graphs.c"
#include "support/capture.c"
#include "support/static.c"
#include "support/aliases.c"
#include "support/scopes.c"
#include "support/qualified.c"

/** Explicitly run every independent graph, capture and failure scenario. */
void run(void) {
    it_admits_actual_associated_test_units_using_retained_physical_ancestry();
    it_normalizes_whole_qualified_operands_before_selecting_constant_members();
    it_keeps_inherited_constant_members_without_fabricating_source_spans();
    it_rejects_invalid_and_forbidden_qualified_namespaces_after_reduction();
    it_unwinds_every_whole_qualified_path_allocation_failure();
    it_checks_standard_scope_isolation_at_actual_alias_use();
    it_checks_real_external_selectors_and_explicit_source_test_dependencies();
    it_reports_actual_missing_wildcard_targets_without_inventing_availability();
    it_resolves_actual_defining_scopes_and_relative_dependencies_without_rewriting_sources();
    it_rejects_actual_scope_collisions_unknown_aliases_and_malformed_profiles();
    it_unwinds_every_actual_scope_and_normalization_allocation_failure();
    it_tracks_local_configuration_presence_and_bytes_without_rereading_disk();
    it_reads_local_configuration_only_for_required_modules_with_owned_errors();
    it_tracks_retained_package_configuration_without_rereading_disk();
    it_invalidates_static_producers_and_selected_consumers_from_nested_sources();
    it_matches_independent_reachability_for_every_three_node_graph_and_changed_seed();
    it_deduplicates_keys_edges_and_rejects_conflicting_or_closed_mutation();
    it_detects_dependency_membership_and_embedded_zero_payload_changes();
    it_invalidates_exact_native_consumers_and_complete_root_configuration();
    it_unwinds_every_capture_allocation_failure_without_sealing_partial_graphs();
    it_owns_the_failed_native_input_path_after_project_destruction();
}
/** Return success only after all assertions complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
