/** @file
 * @brief Accept the synchronous Ama slice through a maintained constructor/method project and real CLI failure compositions.
 */
#include "common.c"
#include "support/verification.c"
#include "support/execution.c"
#include "support/failures.c"

/** Select every project/check/verified-execution acceptance case explicitly. */
void run(void) {
    it_checks_the_constructor_project_and_complete_failure_entries();
    it_runs_independent_captured_methods_and_complete_resource_cleanup();
    it_runs_captured_methods_under_target_address_and_undefined_behavior_sanitizers();
    it_reports_body_failure_without_running_an_acquisition_handler();
    it_does_not_retry_failed_literal_construction();
    it_executes_live_traversal_with_complete_resource_cleanup();
}

/** Return success only after all ordinary and target-sanitized observations pass. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
