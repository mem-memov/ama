/** @file
 * @brief Run real original/raw/provider/closure joins for explicit source-copy modes and refusals.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "destinations.c"
#include "physical.c"
#include "prepared.c"
#include "material.c"
#include "image.c"
#include "image-roots.c"
#include "freshness.c"
#include "units.c"
#include "tests.c"
#include "declared.c"
#include "contexts.c"
#include "environment.c"
#include "permissions.c"
#include "versions.c"
#include "permission-additions/fixture.c"
#include "permission-additions/cases.c"
#include "permission-additions/check.c"
#include "permission-additions/initialize.c"
#include "modes.c"
#include "refusals.c"
#include "data.c"
#include "failures.c"
#include "domains/fixture.c"
#include "domains/projection.c"
#include "domains/proof.c"
#include "domains/cases.c"
/** Explicit runner keeps each accepted preparation boundary independently named. */
void run(void) {
    it_assembles_root_replacement_in_one_projection_and_refuses_invalid_final_roots();
    it_retires_every_simultaneous_root_replacement_allocation_failure();
    it_proves_real_promotion_and_root_relocation_without_physical_source_removal();
    it_retires_registry_scratch_failures_and_refuses_foreign_removal_names();
    it_retires_each_wider_envelope_observation_allocation_failure();
    it_rejects_original_conflicts_in_augmented_root_analysis();
    it_checks_complete_final_permission_sets_and_exact_scope_owners_without_allocation();
    it_initializes_required_empty_version_areas_and_preserves_reserved_identity_classes();
    it_refuses_carried_existing_and_foreign_version_initialization_and_retires_failures();
    it_prepares_new_version_permissions_from_empty_scope_and_passes_actual_final_compiler();
    it_retires_initialized_scope_allocations_and_refuses_existing_or_foreign_environments();
    it_compares_exact_original_observation_facts_without_allocation_or_timestamp_guesses();
    it_revalidates_real_ignored_membership_opaque_bytes_and_live_inode_replacement();
    it_assembles_exact_ancestor_images_and_explicit_replacements_without_host_mutation();
    it_refuses_ambiguous_unobserved_and_outside_image_assembly_and_retires_failures();
    it_joins_every_mapped_member_with_exact_prepared_or_opaque_content();
    it_refuses_unrelated_ambiguous_and_unapplied_material_preparations();
    it_retires_composed_configuration_failures_and_rejects_foreign_authority();
    it_derives_only_authorized_concrete_permission_additions_and_passes_final_compiler_sites();
    it_refuses_uncovered_or_missing_copy_permission_targets_before_publishing_additions();
    it_retires_permission_addition_failures_and_refuses_foreign_baselines();
    it_derives_actual_original_and_copied_selection_roots_without_new_wildcard_matches();
    it_prepares_actual_package_environments_with_exact_version_isolation_and_matches();
    it_retires_every_actual_package_environment_preparation_allocation();
    it_refuses_malformed_empty_version_environments_and_keeps_standard_roots_scope_free();
    it_proves_existing_parent_and_detached_context_omissions_against_real_final_images();
    it_accounts_for_context_omissions_without_losing_active_units_or_failed_prefixes();
    it_validates_complete_explicit_version_placement_tables_without_allocation();
    it_maps_declared_assets_with_explicit_origins_and_preserves_environment_ownership();
    it_refuses_conflicting_shared_declared_input_destinations();
    it_retires_every_declared_material_allocation_and_refuses_invalid_interfaces();
    it_maps_nested_test_material_by_deepest_source_association_across_versions();
    it_maps_complete_test_trees_and_agrees_with_actual_unit_topology();
    it_refuses_test_mapping_authority_conflicts_and_all_allocation_failures();
    it_maps_real_opaque_unit_companions_and_plus_data_with_durable_capture_descriptors();
    it_refuses_unmapped_or_conflicting_unit_causes_and_retires_every_allocation_failure();
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
