/** @file
 * @brief Run actual source-request matrices, rejection boundaries and allocation ownership explicitly.
 * @see [T277 entry review](../../../../../../docs/implementation/source-operations.md#t277-entry-review-after-accepted-t276)
 */
#include "common.c"
#include "cases.c"
#include "refusals.c"
#include "failures.c"
/** Keep independent request laws and resource failure checks visibly runnable. */
void run(void) {
    it_recognizes_explicit_source_operation_modes_and_policies_without_effects();
    it_preserves_root_families_exact_argument_indices_and_order_independent_exclusions();
    it_refuses_incomplete_unknown_repeated_and_conflicting_source_arguments();
    it_refuses_ambiguous_root_scope_and_noncanonical_exclusion_keys();
    it_retires_every_source_request_allocation_and_preserves_repeat_results();
}
/** Adapt the leaf harness to its explicit assertion runner. */
int main(void) { run(); return 0; }
