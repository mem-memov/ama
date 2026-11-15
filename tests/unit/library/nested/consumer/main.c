/** @file
 * @brief Run a public-header-only C host across native-parent and separately compiled child boundaries.
 */
#include "common.c"
#include "ownership.c"
#include "failures.c"
/** Register retained foreign resource ownership and complete partial-failure cleanup. */
void run(void) {
    it_owns_fresh_child_resources_and_copies_only_independent_results();
    it_unwinds_partial_child_resources_before_failed_parent_construction_returns();
    it_unwinds_every_parent_constructor_and_step_allocation_failure();
}
/** Return after every independent C value/resource owner was consumed. */
int main(void) { run(); return 0; }
