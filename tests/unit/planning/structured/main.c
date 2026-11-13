/** @file
 * @brief Compose real source trees into fixed loop labels, selected-action continuations and bounded frame operands.
 */
#include "../common.c"
#include "support/fixture.c"
#include "support/trees.c"
#include "support/loops.c"
#include "support/traversal.c"
#include "support/deep.c"
#include "support/failures.c"
/** Execute each explicit source-to-IR assertion case. */
void run(void) {
    it_lowers_deep_operation_paths_and_traversal_scopes_without_recursive_compiler_calls();
    it_converts_intrinsic_header_results_without_an_intermediate_receiver();
    it_rejects_damaged_loop_frames_chain_roots_and_subtree_continuations();
    it_unwinds_every_complete_structured_lowering_allocation();
    it_repeats_complete_headers_and_releases_protection_before_body_or_exit();
    it_keeps_header_handlers_in_the_incoming_loop_context_after_chain_abandonment();
    it_lowers_foreach_scopes_and_resume_validation_with_fixed_cursor_labels();
    it_keeps_selected_targets_and_caller_operands_through_depth_first_actions();
    it_lowers_list_methods_and_ephemeral_outputs_without_synthetic_module_bodies();
}
/** Return success only after all complete typed-plan cases pass. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
