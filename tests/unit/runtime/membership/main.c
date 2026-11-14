/** @file
 * @brief Run exact checked-membership component scenarios with counted resources.
 */
#include "identity.c"
#include "failures.c"

/** Execute the independent membership laws, preparation failures and boundaries. */
void run(void) {
    it_distinguishes_duplicate_targets_and_rejects_reused_occurrences();
    it_preserves_record_addresses_and_bijections_through_growth_and_edits();
    it_replaces_reference_memberships_atomically_and_resets_incarnations();
    it_permanently_retires_exhausted_record_generations();
    it_preserves_empty_state_and_output_on_every_initial_allocation_failure();
    it_preserves_live_occurrences_on_failed_replacement_and_invalid_bounds();
}

/** Report success only after all invariants and allocation-credit checks pass. */
int main(void) { run(); return EXIT_SUCCESS; }
