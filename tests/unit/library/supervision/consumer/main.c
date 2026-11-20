/** @file
 * @brief Run the public-only host of independently suspended parent and child Ama computations.
 */
#include "common.c"
#include "continuations.c"
#include "supervision.c"
#include "failures.c"
#include "peers.c"
/** Register suspension, isolation, supervision, explicit reset and own-failure boundaries. */
void run(void) {
    it_preserves_independent_peers_during_owned_child_cleanup();
    it_returns_all_c_frames_before_each_explicit_outer_yield();
    it_copies_child_reports_and_requires_explicit_reset_after_supervised_failure();
    it_propagates_child_failure_only_when_the_parent_policy_requests_it();
    it_keeps_own_report_allocation_failures_technical();
    reset_fixture(); ama_nested_probe_v1_set(NULL);
}
/** Return only after every active code/resource dependency was retired. */
int main(void) { run(); return 0; }
