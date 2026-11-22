/** @file
 * @brief Exercise source/test least closure through real files, source scopes and compiler checks.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../../../../compiler/common.c"
#include "../../../../../../tools/project/acquisition/closure/plan/plan.h"
#include "delivery/source/export.h"
#include "delivery/source/entry.h"
#include "source/path/logical.h"
#include <unistd.h>
#include "support/prepared.c"
#include "cases/structural.c"
#include "support/empty-version.c"
#include "cases/whole-version.c"
#include "cases/drafts.c"
#include "cases/dependency-policy.c"
#include "cases/allocations.c"
#include "cases/declared-modules.c"
#include "cases/types-and-requests.c"
#include "cases/standard-native.c"
/** Run real source/test/context composition and refusal partitions explicitly. */
void run(void) {
    it_closes_child_only_delivery_without_activating_its_draft_parent();
    it_preserves_external_dependency_membership_without_requiring_unchanged_parent_tests();
    it_retains_selected_draft_units_and_active_children_without_invented_parent_requirements();
    it_refuses_contradictory_draft_definition_and_test_association_proofs();
    it_refuses_a_draft_marker_on_selected_version_scope();
    it_promotes_context_and_closes_real_source_cycles_with_all_test_support();
    it_refuses_publication_after_independent_test_admission_failure();
    it_retains_explicit_declared_test_inputs_without_guessing_package_assets();
    it_refuses_an_associated_directory_containing_only_open_test_helpers();
    it_retains_explicit_empty_versions_without_inventing_module_instances();
    it_closes_whole_versions_through_their_actual_source_and_test_units();
    it_retires_every_controller_allocation_failure_in_real_compositions();
    it_promotes_declared_module_cycles_once_independently_of_semantic_dependency_policy();
    it_refuses_invalid_declared_module_identities_and_provenance_without_promotion();
    it_retires_every_controller_allocation_in_declared_data_promotion_and_cycles();
    it_handles_empty_and_invalid_selection_requests_without_provider_effects();
    it_retains_injected_test_types_through_their_original_test_scope();
    it_admits_tests_whose_original_formals_close_from_typed_defaults();
    it_retains_standard_native_inputs_without_inventing_package_scope_or_tests();
}
/** Finish only after every plan/provider/compiler source owner retires. */
int main(void) { run(); return 0; }
