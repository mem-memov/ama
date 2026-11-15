/** @file
 * @brief Execute copied continuation enrollment and explicit external resource consumption without granting implicit callback rights.
 */
#include "../execution/common.c"

#include "support/fixture.c"
#include "support/copies.c"
#include "support/capabilities.c"
#include "support/failure.c"

/** Run the actual copy, enrollment and external-consumption boundaries. */
void run(void) {
    it_requires_fresh_enrollment_before_dispatching_a_copied_continuation();
    it_preserves_explicitly_shared_external_ports_without_duplicating_membership();
    it_preserves_single_consumer_rights_when_a_native_token_is_copied();
    it_requires_independent_native_waits_despite_shared_external_operation_references();
    it_rejects_copying_enrollment_owners_and_inflight_native_operations();
    it_detaches_every_wait_before_reporting_a_context_wide_technical_failure();
}
/** Return success after each composed program satisfies its exact observable contract. */
int main(void) { run(); return EXIT_SUCCESS; }
