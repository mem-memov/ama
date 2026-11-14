/** @file
 * @brief Compile direct-result trees and observe persistent identity, order and continuation behavior.
 */
#include "../execution/common.c"
#include "support/state.c"
#include "support/selection.c"
#include "support/resume.c"
#include "support/protection.c"

/** Run separately observable tree contracts through the real C toolchain. */
void run(void) {
    it_executes_retained_owned_methods_depth_first_without_cloning();
    it_follows_returned_references_and_uses_caller_indices();
    it_keeps_selected_index_protocol_failures_technical();
    it_resumes_returned_continuations_without_suspending_the_chain();
    it_protects_the_parent_until_a_refused_link_abandons_the_chain();
}

/** Return success only after every compiled composition assertion passes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
