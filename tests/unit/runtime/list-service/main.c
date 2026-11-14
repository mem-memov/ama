/** @file
 * @brief Run list grant and temporary method-result component contracts explicitly.
 */
#include "authority.c"
#include "results.c"

/** Run independent authority, lifetime, failure and stable-storage laws. */
void run(void) {
    it_keeps_siblings_busy_and_borrows_only_the_exact_parent_grant();
    it_retains_selected_owner_protection_and_rejects_alias_reentry();
    it_finalizes_owned_results_once_and_expires_escaped_handles();
    it_keeps_reference_results_nonowning_and_preserves_target_activity();
    it_cleans_failed_results_without_repeating_destruction_or_leaking_grants();
    it_borrows_only_aligned_bounded_lexical_frame_slots();
}

/** Return success after every assertion and counted-resource teardown completes. */
int main(void) { run(); return EXIT_SUCCESS; }
