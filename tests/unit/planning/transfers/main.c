/** @file
 * @brief Verify source protection, explicit commit/release groups and invocation-mode exclusions through real typed projects.
 */
#include "../common.c"
#include "support/fixture.c"
#include "support/order.c"
#include "support/failures.c"
/** Execute named composition and failed-allocation cases. */
void run(void) {
    it_preserves_assignment_and_reference_output_modes_without_snapshot_sources();
    it_lowers_source_protection_and_individual_commits_in_the_three_required_orders();
    it_omits_entry_preparation_for_resume_retained_and_constant_reads();
    it_rejects_damaged_replacement_recipes_and_partial_commit_groups();
    it_unwinds_every_ordered_transfer_lowering_allocation();
}
/** Return success only after the whole independent transfer suite passes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
