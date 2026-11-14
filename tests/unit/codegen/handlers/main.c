/** @file
 * @brief Compile deep local refusal handlers, committed prefixes, caller boundaries and suspension.
 */
#include "common.c"
#include "support/progress.c"
#include "support/boundaries.c"
#include "support/suspension.c"

/** Execute each independently observable handler contract with real generated programs. */
void run(void) {
    it_unwinds_deep_handlers_in_caller_scope_and_preserves_prior_progress();
    it_accepts_empty_handlers_after_releasing_the_complete_tree();
    it_does_not_offer_a_descendant_refusal_to_ancestor_handlers();
    it_keeps_unrelated_outer_callers_when_a_method_handles_its_own_refusal();
    it_keeps_selected_body_failures_technical_at_every_handler_level();
    it_resumes_a_yielded_handler_without_restoring_abandoned_actions();
}

/** Return success only after all composition assertions and cleanup complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
