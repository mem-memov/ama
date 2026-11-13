/** @file
 * @brief Verify typed selected result identities, caller operands, ordered actions, handler roots and allocation failures.
 */
#include "../typed-common.c"
#include "support/fixture.c"
#include "support/actions.c"
#include "support/modes.c"
#include "support/failures.c"
/** Run every selected result context and failure partition explicitly. */
void run(void) {
    it_retains_selected_field_identity_and_ordered_sibling_actions_without_cloning();
    it_starts_handler_calls_in_the_caller_instead_of_the_selected_interface();
    it_accepts_each_parent_child_mode_and_scalar_reference_indexed_form();
    it_rejects_selector_fallback_wrong_roles_forms_and_caller_index_capabilities();
    it_resolves_deep_reference_result_trees_iteratively();
    it_unwinds_each_allocation_in_selected_operations_and_action_links();
}
/** Return success only after all selected-tree assertions complete. */
int main(void) { run(); return EXIT_SUCCESS; }
