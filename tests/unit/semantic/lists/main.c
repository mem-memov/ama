/** @file
 * @brief Verify typed list service scopes and ephemeral results without hidden module instances or action constants.
 */
#include "../typed-common.c"
#include "support/fixture.c"
#include "support/methods.c"
#include "support/capabilities.c"
#include "support/failures.c"
/** Run the fixed interfaces, target restrictions, copy partitions, integer profiles and allocation failures explicitly. */
void run(void) {
    it_specializes_all_six_method_interfaces_without_fabricated_declarations();
    it_types_ephemeral_results_and_services_selected_from_module_results();
    it_rejects_irrelevant_arguments_invalid_method_modes_and_retired_interfaces();
    it_requires_element_copy_only_for_explicit_owning_append_insert_sources();
    it_requires_the_exact_standard_native_integer_profile_and_bridge_obligation();
    it_unwinds_every_list_interface_binding_and_result_allocation();
}
/** Return success only after each explicit semantic assertion completes. */
int main(void) { run(); return EXIT_SUCCESS; }
