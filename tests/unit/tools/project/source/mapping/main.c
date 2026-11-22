/** @file
 * @brief Check exact clone relation injectivity, nominal distinctions, owned strings and atomic refusal.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../selection/common.c"
#include "../../../../../../tools/project/source/mapping/map.h"
static SourceCloneMap map;
static const char *const paths[] = {"/mapped/a", "/mapped/b"};
static const char *const names[] = {"/vendor/target/v2.0.0/src/a", "/vendor/target/v2.0.0/src/b"};
/** Borrow actual original declarations; identical source text never substitutes for handle ownership. */
static const ModuleSource *original(const char *identity) {
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&capture.catalog, identity, &source) == MODULE_OK);
    return source;
}
/** Independent nominal/path equality oracle exhausts every pair of two-key placements. */
static void it_matches_all_two_origin_two_path_two_name_pair_constraints(void) {
    for (unsigned same_origin = 0; same_origin < 2; same_origin++) {
        for (size_t path = 0; path < 2; path++) {
            for (size_t name = 0; name < 2; name++) {
                prepare(); source_clone_map_init(&map, sources.allocator, &capture);
                const ModuleSource *first = original("/vendor/pkg/v1.0.0/src/parent/child");
                const ModuleSource *second = same_origin ? first : original("/vendor/pkg/v1.0.0/src/parentish");
                const SourceCloneMapping *entry = NULL;
                assert(source_clone_map_add(&map, first, paths[0], names[0], &entry) == SOURCE_CLONE_OK);
                const SourceCloneMapping *saved = entry;
                bool allowed = same_origin ? path == 0 && name == 0 : path == 1 && name == 1;
                SourceCloneStatus status = source_clone_map_add(&map, second, paths[path], names[name], &entry);
                assert(status == (allowed ? SOURCE_CLONE_OK : SOURCE_CLONE_CONFLICT));
                if (!allowed) {
                    assert(entry == saved && map.conflict == saved && !map.ready);
                    assert(source_clone_map_seal(&map) == SOURCE_CLONE_CONFLICT);
                    assert(source_clone_map_add(&map, second, paths[1], names[1], &entry) == SOURCE_CLONE_CONFLICT);
                } else { assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK && map.ready); }
                source_clone_map_destroy(&map); finish();
            }
        }
    }
}
/** Destination strings are owned, repeat proposals allocate nothing, and seal freezes the relation. */
static void it_owns_placement_strings_reuses_equal_proposals_and_freezes_after_seal(void) {
    prepare(); source_clone_map_init(&map, sources.allocator, &capture);
    const ModuleSource *source = original("/vendor/pkg/v1.0.0/src/parent/child");
    char path[] = "/mapped/a", name[] = "/vendor/target/v2.0.0/src/a";
    const SourceCloneMapping *entry = NULL;
    assert(source_clone_map_add(&map, source, path, name, &entry) == SOURCE_CLONE_OK);
    size_t before = allocation_count; const SourceCloneMapping *saved = entry;
    assert(source_clone_map_add(&map, source, path, name, &entry) == SOURCE_CLONE_OK);
    assert(entry == saved && allocation_count == before);
    path[1] = 'X'; name[1] = 'X';
    assert(!strcmp(entry->path, paths[0]) && !strcmp(entry->identity, names[0]));
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_INVALID && map.ready);
    assert(source_clone_map_add(&map, source, paths[0], names[0], &entry) == SOURCE_CLONE_INVALID && map.ready);
    assert(allocation_count == before); source_clone_map_destroy(&map); finish();
}
/** Equal-body declarations remain distinct original types and cannot share a copied identity. */
static void it_preserves_nominal_source_distinctions_even_when_bytes_are_identical(void) {
    prepare(); source_clone_map_init(&map, sources.allocator, &capture);
    const ModuleSource *first = original("/vendor/pkg/v1.0.0/src/parent/child");
    const ModuleSource *second = original("/vendor/pkg/v1.0.0/src/parentish");
    assert(source_length(first->text) == source_length(second->text)
        && !memcmp(source_bytes(first->text), source_bytes(second->text), source_length(first->text)));
    const SourceCloneMapping *entry = NULL;
    assert(source_clone_map_add(&map, first, paths[0], names[0], &entry) == SOURCE_CLONE_OK);
    assert(source_clone_map_add(&map, second, paths[1], names[0], &entry) == SOURCE_CLONE_CONFLICT);
    source_clone_map_destroy(&map); finish();
}
/** Every proposal allocation failure leaves the prior relation intact and prevents sealing. */
static void it_preserves_owned_prefix_and_output_at_every_proposal_allocation_failure(void) {
    size_t count = 0;
    for (size_t failure = 0; !failure || failure <= count; failure++) {
        prepare(); source_clone_map_init(&map, sources.allocator, &capture);
        const ModuleSource *first = original("/vendor/pkg/v1.0.0/src/parent/child");
        const ModuleSource *second = original("/vendor/pkg/v1.0.0/src/parentish");
        const SourceCloneMapping *entry = NULL;
        assert(source_clone_map_add(&map, first, paths[0], names[0], &entry) == SOURCE_CLONE_OK);
        const SourceCloneMapping *saved = entry; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceCloneStatus status = source_clone_map_add(&map, second, paths[1], names[1], &entry);
        if (!failure) { assert(status == SOURCE_CLONE_OK); count = allocation_count - before; assert(count); }
        else {
            assert(status == SOURCE_CLONE_MEMORY && entry == saved && map.first == saved && !saved->next);
            assert(source_clone_map_seal(&map) == SOURCE_CLONE_MEMORY && !map.ready);
        }
        failure_at = 0; source_clone_map_destroy(&map); finish();
    }
}
/** Invalid handles/keys cannot publish rows; empty relations remain sealable. */
static void it_rejects_foreign_or_invalid_proposals_and_accepts_an_empty_complete_relation(void) {
    for (size_t kind = 0; kind < 7; kind++) {
        prepare(); source_clone_map_init(&map, sources.allocator, &capture);
        const ModuleSource *source = original("/vendor/pkg/v1.0.0/src/parent/child");
        ModuleSource foreign = *source;
        const char *path = kind == 1 ? "relative" : kind == 2 ? "/" : kind == 3 ? "/mapped/" : paths[0];
        const char *name = kind == 4 ? "./relative" : kind == 5 ? "/vendor/target/v2.0.0" : names[0];
        SourceCloneMapping sentinel = {0}; const SourceCloneMapping *entry = &sentinel;
        assert(source_clone_map_add(&map, kind == 0 ? &foreign : source, path, name, kind == 6 ? NULL : &entry) == SOURCE_CLONE_INVALID);
        assert(entry == &sentinel && !map.first && source_clone_map_seal(&map) == SOURCE_CLONE_INVALID);
        source_clone_map_destroy(&map); finish();
    }
    prepare(); source_clone_map_init(&map, sources.allocator, &capture);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK && map.ready && !map.first);
    source_clone_map_destroy(&map); finish();
}
#include "physical-cases.c"
#include "version-cases.c"
#include "test-directory-cases.c"
#include "tree-cases.c"
#include "identity-cases.c"
#include "propose-cases.c"
#include "files-cases.c"
#include "correspondence-cases.c"
#include "bindings-cases.c"
#include "sites-cases.c"
#include "operand-cases.c"
/** Run finite injectivity, identity ownership, publication and failure boundaries. */
void run(void) {
    it_matches_common_catalog_for_every_two_ancestor_final_topology_in_src_and_tests();
    it_refuses_invalid_main_indexes_and_preserves_owned_physical_outputs();
    it_composes_version_demands_additional_suffixes_and_injective_clone_proposals();
    it_preserves_test_helper_suffixes_and_refuses_invalid_additional_candidates();
    it_matches_all_three_unit_version_demand_graphs_and_seed_assignments();
    it_owns_version_assignments_and_retires_every_allocation_failure();
    it_refuses_invalid_version_demand_graphs_without_allocating();
    it_composes_catalog_clone_rows_with_shared_logical_test_associations();
    it_maps_associated_tests_by_deepest_directory_independently_of_order();
    it_refuses_invalid_conflicting_and_failed_test_directory_projections();
    it_matches_all_two_origin_two_path_two_name_pair_constraints();
    it_owns_placement_strings_reuses_equal_proposals_and_freezes_after_seal();
    it_preserves_nominal_source_distinctions_even_when_bytes_are_identical();
    it_preserves_owned_prefix_and_output_at_every_proposal_allocation_failure();
    it_rejects_foreign_or_invalid_proposals_and_accepts_an_empty_complete_relation();
    it_places_module_stems_companions_and_subtrees_with_a_consistent_clone_relation();
    it_places_bare_and_empty_directories_at_the_exact_requested_location();
    it_retires_every_explicit_tree_placement_allocation_failure();
    it_refuses_unadmitted_tree_domains_and_invalid_module_destination_forms();
    it_projects_existing_source_names_exactly_as_common_catalog_discovery();
    it_decodes_physical_layouts_without_repairing_invalid_or_excluded_components();
    it_refuses_ambiguous_root_interpretations_independently_of_order();
    it_retires_every_candidate_identity_allocation_and_invalid_input_without_publication();
    it_derives_every_explicit_clone_from_its_placed_main_and_preserves_bare_tree_selection();
    it_refuses_cross_capture_unplaced_or_conflicting_tree_proposals();
    it_retires_every_joined_tree_proposal_allocation_without_partial_acceptance();
    it_checks_every_two_input_output_shape_and_byte_overlap_partition();
    it_rejects_final_file_ancestors_without_confusing_component_prefixes();
    it_joins_complete_explicit_tree_file_maps_with_owned_keys_and_no_opaque_decoding();
    it_retires_every_file_map_allocation_and_invalid_capture_without_acceptance();
    it_requires_both_maps_to_cover_exactly_the_same_original_mains_and_observations();
    it_checks_mapping_correspondence_only_for_complete_relations_without_allocating();
    it_preserves_simultaneous_cycle_bindings_for_every_clone_and_survivor_subset();
    it_refuses_each_changed_clone_edge_field_and_surviving_original_redirection();
    it_refuses_missing_extra_and_survivor_clone_identity_collisions();
    it_checks_binding_laws_without_allocating_and_rejects_invalid_owners();
    it_refuses_retained_targets_removed_by_replacement_even_when_the_final_name_matches();
    it_accepts_identity_preserving_mapped_self_references_after_original_removal();
    it_requires_exact_prepared_site_provenance_after_binding_admission();
    it_prepares_rooted_copied_and_retained_targets_without_rewriting_outside_operand_bytes();
    it_keeps_fixed_dependencies_without_allocation_and_refuses_changed_fixed_identities();
    it_retires_every_rooted_operand_allocation_failure_and_rejects_invalid_handoffs();
}
/** Retire proposal and original owners before successful leaf cleanup. */
int main(void) { run(); return 0; }
