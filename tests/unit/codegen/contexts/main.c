/** @file
 * @brief Independently compile and run isolated generated context ownership, admission and actual parallel execution.
 */
#include "../execution/common.c"
#include "support/cases.c"
/** Execute host/context composition cases through the real compiler/toolchain. */
void run(void) {
    it_interleaves_independent_generated_contexts_with_private_constants_and_pools();
    it_executes_two_generated_contexts_in_parallel_and_refuses_same_host_reentry();
    it_distinguishes_busy_native_destruction_from_refused_host_disposal();
}
/** Return success only after all explicit cases complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
