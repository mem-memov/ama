/** @file
 * @brief Compile and execute maintained generic algorithms over the existing intrinsic list foundation.
 */
#include "../execution/common.c"
#include "support/fixture.c"
#include "support/owned.c"
#include "support/refusals.c"
#include "support/references.c"
#include "support/invalid.c"
/** Register real compiled algorithm observations explicitly. */
void run(void) {
    it_rejects_invalid_algorithm_categories_storage_and_copy_capabilities();
    it_preserves_missing_reference_refusal_for_concrete_algorithm_types();
    it_reverses_reference_memberships_without_copying_or_destroying_targets();
    it_executes_nested_partial_list_composition_and_independent_instances();
    it_mirrors_and_drains_owned_lists_with_live_boundary_correction();
    it_executes_the_exact_maintained_standalone_example();
    it_reverses_explicitly_typed_empty_lists_without_sample_elements();
}
/** Return only after compiled behavior and fixture owner teardown pass. */
int main(void) { run(); return 0; }
