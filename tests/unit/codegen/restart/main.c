/** @file
 * @brief Compile entry/resume interface semantics, explicit host outcomes and standalone yield cleanup.
 */
#include "../execution/common.c"
#include "support/entry.c"
#include "support/host.c"
#include "support/launcher.c"

/** Execute each invocation/host/launcher boundary explicitly. */
void run(void) {
    it_restarts_entry_defaults_without_resetting_body_fields_or_resume_inputs();
    it_preserves_healthy_hosts_across_resume_rejection_and_explicit_lifecycle();
    it_keeps_admitted_execution_failure_terminal_and_never_resumes_it();
    it_rejects_unhandled_standard_root_yield_after_nonexecuting_cleanup();
}

/** Return success after all compiled cases and teardown checks complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
