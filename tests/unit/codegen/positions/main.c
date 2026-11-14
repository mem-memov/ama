/** @file
 * @brief Compile exact statement resumption, immediate-caller yield routing and availability boundaries.
 */
#include "../execution/common.c"
#include "support/callers.c"
#include "support/branch.c"

/** Run each source-to-runtime saved-position contract explicitly. */
void run(void) {
    it_returns_yields_to_waiting_callers_and_releases_callees_for_resume();
    it_refuses_resume_after_the_last_saved_segment_completes();
    it_resumes_the_saved_branch_statement_after_external_condition_change();
}

/** Return success only after all generated-program assertions and cleanup pass. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
