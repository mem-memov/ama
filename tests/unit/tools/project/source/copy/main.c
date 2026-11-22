/** @file
 * @brief Run real original/raw/provider/closure joins for explicit source-copy modes and refusals.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "destinations.c"
#include "physical.c"
#include "prepared.c"
#include "modes.c"
#include "refusals.c"
#include "data.c"
#include "failures.c"
/** Explicit runner keeps each accepted preparation boundary independently named. */
void run(void) {
    it_joins_actual_copy_placement_and_projected_version_with_final_compiler_sites();
    it_checks_copy_main_placement_topology_explicit_agreement_and_failure_sealing();
    it_retires_every_actual_copy_main_placement_allocation_failure();
    it_selects_deepest_original_test_owners_and_refuses_shared_helper_version_conflicts();
    it_handles_standard_empty_and_draft_destination_partitions();
    it_composes_actual_copy_closures_with_logical_and_physical_destination_candidates();
    it_retires_every_actual_copy_destination_allocation_failure();
    it_refuses_invalid_copy_destination_inputs_and_namespace_crossings();
    it_composes_every_copy_dependency_mode_with_actual_cycles_tests_and_standard_targets();
    it_prepares_explicit_standard_and_draft_tree_selections_without_invented_tests();
    it_preserves_empty_raw_material_and_checks_original_ownership_before_empty_closure();
    it_refuses_missing_and_only_open_required_tests_using_actual_compiler_admission();
    it_refuses_ignored_version_selection_before_structural_preparation_or_test_probes();
    it_refuses_invalid_copy_selection_and_mode_interfaces_without_provider_effects();
    it_includes_declared_module_data_in_default_copy_mode_with_its_real_tests();
    it_refuses_changed_original_unit_bytes_and_new_companions_before_copy_acceptance();
    it_retires_every_copy_owner_allocation_for_default_dependency_and_declared_data_compositions();
}
/** Cases retire copy borrowers before original source/compiler fixture owners. */
int main(void) { run(); return 0; }
