/** @file
 * @brief Check immediate inferred types, source-order classification and nonshadowing foreach scopes.
 */
#include "../typed-common.c"
#include "support/fixture.c"
#include "support/assignments.c"
#include "support/results.c"
#include "support/scopes.c"
#include "support/failures.c"
/** Run every inference shape, ordering, scope and failure boundary explicitly. */
void run(void) {
    it_types_intrinsic_list_results_without_action_constants_or_method_instances();
    it_types_a_first_assignment_receiver_without_synthetic_declaration_or_initializer();
    it_derives_the_complete_assignment_source_storage_table_immediately();
    it_types_output_receivers_from_entry_resume_retained_and_constant_interfaces();
    it_orders_interleaved_selected_results_and_ordinary_bindings_by_written_position();
    it_rejects_earlier_unresolved_uses_self_dependencies_and_later_explicit_collisions();
    it_scopes_disjoint_local_references_without_retroactive_module_binding();
    it_reuses_enclosing_or_later_explicit_references_without_shadowing();
    it_rejects_incompatible_bindings_and_construction_time_local_access();
    it_unwinds_every_inference_allocation_failure();
}
/** Return success only after all explicit assertion cases complete. */
int main(void) { run(); return EXIT_SUCCESS; }
