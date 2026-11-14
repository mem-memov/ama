/** @file
 * @brief Compile continuation cleanup, native resource clone failure and synchronous native boundary compositions.
 */
#include "../execution/common.c"
#include "support/resources.c"
#include "support/mixed.c"
#include "support/native.c"

/** Select static/native boundaries before compiled mixed-state and failure cases. */
void run(void) {
    it_rejects_ama_yield_inside_a_native_declaration();
    it_preserves_mixed_completed_and_yielded_owned_positions();
    it_unwinds_native_clone_prefixes_preserves_old_values_and_disposes_failed_contexts();
    it_keeps_native_status_failure_distinct_from_yield_and_admission();
}

/** Return success after all explicit construction, copy, control and cleanup assertions. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
