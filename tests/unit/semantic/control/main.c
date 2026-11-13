/** @file
 * @brief Verify exact control roots and loop ownership independently of later execution-frame lowering.
 */
#include "../typed-common.c"
#include "semantic/internal.h"
#include "support/fixture.c"
#include "support/roots.c"
#include "support/scopes.c"
#include "support/failures.c"
/** Run complete condition, traversal, warning, lexical-exit and allocation-failure partitions. */
void run(void) {
    it_retains_the_complete_restart_root_and_exact_handler_break_targets();
    it_resolves_all_header_modes_and_indexed_terminal_results_in_caller_context();
    it_types_intrinsic_terminal_conditions_without_fake_field_accesses();
    it_retains_branch_polarity_and_syntactic_empty_body_warnings();
    it_distinguishes_foreach_binding_from_indexed_truth_and_reuses_exact_lexical_cells();
    it_checks_skipped_paths_predicate_capabilities_and_header_handler_exits();
    it_unwinds_every_allocation_in_complete_control_contexts();
}
/** Return success only after all source-owned semantic assertions complete. */
int main(void) { run(); return EXIT_SUCCESS; }
