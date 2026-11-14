/** @file
 * @brief Compile exact loop-header roots, release boundaries, exits, failures and suspension.
 */
#include "common.c"
#include "support/repeat.c"
#include "support/empty.c"
#include "support/refusals.c"
#include "support/failure.c"
#include "support/exits.c"
#include "support/suspension.c"
#include "support/selection.c"

/** Run each distinct control contract with actual generated C/native execution. */
void run(void) {
    native_hook_override = observe_condition;
    it_repeats_the_complete_header_and_releases_before_body_and_exit();
    it_keeps_nested_loop_and_branch_repeat_boundaries_independent();
    it_checks_empty_named_and_header_loops_once_for_both_truth_values();
    it_handles_refusal_at_each_header_depth_without_running_the_loop();
    it_keeps_terminal_conversion_failure_outside_refusal_handlers();
    it_exits_nested_loops_and_returns_from_header_bodies();
    it_resumes_inside_the_body_before_repeating_the_header();
    it_selects_an_indexed_terminal_condition_using_the_caller_index_once();
    native_hook_override = NULL;
}

/** Return success only after all explicit loop compositions and cleanup finish. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
