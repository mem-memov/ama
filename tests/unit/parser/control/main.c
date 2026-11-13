/** @file
 * @brief Verify structural control recognition independently from declarations and types.
 */
#include <stdio.h>
#include "../ast-common.c"
/** Require valid syntax while retaining the fixture for exact AST and source checks. */
static AstNode *parsed(const char *text) {
    ParseStatus status = parse_text(text);
    if (status != PARSE_OK) { fprintf(stderr, "%s: %s\n", text, module.error.diagnostic.message); }
    assert(status == PARSE_OK && module.valid);
    return module.body.first;
}
#include "support/branches.c"
#include "support/loops.c"
#include "support/boundaries.c"
/** Execute every control family and ownership/error boundary explicitly. */
void run(void) {
    it_distinguishes_written_positive_negative_and_omitted_bodies();
    it_counts_declarations_as_statements_and_keeps_warning_marker_order();
    it_folds_the_complete_condition_path_into_one_lexical_loop();
    it_distinguishes_owned_indices_from_traversal_reference_binders();
    it_uses_incoming_loop_context_for_header_handlers();
    it_rejects_nonadjacent_markers_nonlinear_headers_and_reopened_regions();
    it_handles_deep_control_frames_and_all_allocation_failures();
}
/** Succeed only after all assertion cases finish. */
int main(void) { run(); return EXIT_SUCCESS; }
