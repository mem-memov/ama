/**
 * @file
 * @brief Check direct result trees independently of declared interfaces or runtime availability.
 */
#include <stdio.h>
#include "../ast-common.c"

/** Parse a source fixture and require a single ordinary root operation. */
static AstNode *root(const char *text) {
    ParseStatus status = parse_text(text);
    if (status != PARSE_OK) { fprintf(stderr, "Result tree: %s\n", module.error.diagnostic.message); }
    assert(status == PARSE_OK && module.valid);
    assert(module.body.first != NULL && module.body.first->kind == AST_CALL);
    assert(module.body.first->next == NULL);
    return module.body.first;
}
#include "support/actions.c"
#include "support/handlers.c"
#include "support/depth.c"

/** Execute every ordered tree, context, depth and failure scenario explicitly. */
void run(void) {
    it_builds_the_short_direct_result_form_without_an_intermediate_receiver();
    it_keeps_sibling_action_order_and_binding_style_independent_of_selectors();
    it_parses_unmarked_reads_and_whole_list_method_parents_as_operation_trees();
    it_attaches_refusal_markers_to_their_exact_operations_and_resets_handler_selector_context();
    it_rejects_input_after_selection_and_misindented_or_ineligible_handlers();
    it_parses_deep_result_paths_without_recursive_c_activation();
    it_unwinds_every_allocation_failure_in_a_result_tree_and_handler();
}
/** Return success only after every assertion completes. */
int main(void) { run(); return EXIT_SUCCESS; }
