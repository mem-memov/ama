/** @file
 * @brief Exercise independent context/instance storage planning and exact lexical frame identities.
 */
#include "../common.c"
#include "planning/internal.h"
#include "support/fixture.c"
#include "support/domains.c"
#include "support/failures.c"
#include "support/deep.c"
/** Execute named assertions for storage domain separation and failure cleanup. */
void run(void) {
    it_bounds_deep_chain_and_traversal_storage_by_lexical_sites();
    it_keeps_intrinsic_results_in_caller_frames_without_method_instances();
    it_preserves_deterministic_storage_across_independent_plan_owners();
    it_separates_context_permanent_interface_and_fixed_lexical_storage();
    it_retains_the_general_fallback_and_keeps_shared_namespaces_out_of_pools();
    it_rejects_incomplete_or_duplicated_logical_storage_without_allocating();
    it_unwinds_every_storage_frame_cell_and_origin_allocation();
}
/** Return success after every named storage case passes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
