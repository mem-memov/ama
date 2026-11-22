/** @file
 * @brief Run physical removal material, declaration closure, original review and failure compositions explicitly.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "cases/material.c"
#include "cases/declarations.c"
#include "cases/declared-observations.c"
#include "cases/review.c"
#include "cases/failures.c"
/** Run actual missing/present test material, exact original cuts, overlap and allocation ownership. */
void run(void) {
    it_refuses_unowned_or_malformed_declared_input_observations_before_material_expansion();
    it_refuses_declared_observations_that_change_known_original_main_companion_or_configuration_bytes();
    it_refuses_new_companion_presence_and_known_directory_to_file_contradictions();
    it_builds_removal_sets_with_missing_tests_and_preserves_bare_directory_ancestors();
    it_includes_actual_tests_and_reports_all_outside_graph_input_and_client_blockers();
    it_merges_overlapping_raw_and_associated_test_material_without_duplicate_keys();
    it_refuses_changed_associated_original_observations();
    it_accepts_support_only_tests_and_empty_directory_removal_without_executable_admission();
    it_refuses_unadmitted_domains_and_keeps_review_output_atomic_on_invalid_clients();
    it_includes_selected_explicit_assets_and_preserves_shared_outside_consumers();
    it_checks_conflicting_unowned_and_module_role_declared_inputs();
    it_retires_every_explicit_asset_material_allocation_failure();
    it_closes_module_asset_cycles_once_and_leaves_outgoing_dependency_units_untouched();
    it_protects_original_native_and_configuration_inputs_without_treating_guidance_as_compilation();
    it_derives_shared_original_declaration_uses_for_surviving_owners();
    it_protects_the_selected_package_environment_for_all_surviving_original_modules();
    it_reviews_standard_material_without_invented_ordinary_configuration_requirements();
    it_retires_all_material_and_review_allocation_failures();
}
/** Complete only after all original, selected and review owners retire. */
int main(void) { run(); return 0; }
