/** @file
 * @brief Compile retained and immutable result reads with independent state, continuation, refusal and partial-progress observations.
 */
#include "../execution/common.c"
#include "support/state.c"
#include "support/constants.c"
#include "support/refusals.c"
#include "support/progress.c"

/** Run each separately observable read contract through the actual frontend, C toolchain and runtime. */
void run(void) {
    it_copies_retained_module_state_without_executing_resetting_or_copying_saved_positions();
    it_reads_qualified_and_namespace_constants_without_constructing_or_executing_their_modules();
    it_refuses_a_busy_retained_target_before_changing_its_receiver();
    it_refuses_a_stale_retained_target_before_changing_its_receiver();
    it_keeps_retained_results_and_the_successful_prefix_when_a_later_output_fails();
}

/** Return success only after every explicit read-composition case completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
