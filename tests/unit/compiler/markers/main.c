/** @file
 * @brief Run complete marker identity, storage admission and resource-failure compositions.
 */
#include "support/identity.c"
#include "support/storage.c"

/** Explicit case list ensures every marker property actually executes. */
void run(void) {
    it_forwards_distinct_marker_types_without_runtime_layout();
    it_preserves_root_kind_in_marker_identity();
    it_refuses_marker_runtime_binding_and_scope_handles();
    it_rejects_direct_and_exported_marker_storage_in_every_form();
    it_rejects_marker_producer_selection_application_and_constant_reads();
    it_rejects_marker_executable_roots();
    it_rejects_substituted_marker_fields_and_uninstantiated_definitions();
    it_unwinds_every_marker_frontend_allocation_refusal();
}

/** Return success only after assertions and counted teardown complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
