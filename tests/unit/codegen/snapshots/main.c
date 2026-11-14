/** @file
 * @brief Compile explicit continuation snapshots with whole-tree eligibility, distinct identities and independent service state.
 */
#include "common.c"
#include "support/trees.c"
#include "support/availability.c"
#include "support/identity.c"

/** Execute every snapshot boundary and compare actual native effects. */
void run(void) {
    it_copies_owned_tree_positions_explicitly_and_clears_ordinary_positions_and_histories();
    it_snapshots_available_owned_sources_with_busy_external_reference_targets();
    it_rejects_a_waiting_owned_descendant_before_any_clone_callback();
    it_rejects_continuation_copy_to_the_same_actual_identity_through_an_alias();
}

/** Return success only after all compiled snapshot cases and cleanup complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
