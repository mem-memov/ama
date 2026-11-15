/** @file
 * @brief Observe explicit enrollment, external references and single-consumer completion rights.
 */
/** A copied position is inert until explicitly adopted and enqueued under a new noncopyable owner. */
static void it_requires_fresh_enrollment_before_dispatching_a_copied_continuation(void) {
    boundary_schedule("adopt", "1:1:0\n2:2:0\n2:2:0\n2:2:0\n");
}
/** External cancellation references remain shared while owned execution state and scheduling identities are independent. */
static void it_preserves_explicitly_shared_external_ports_without_duplicating_membership(void) {
    boundary_schedule("shared-port", "1:1:0\n99:2:0\n99:2:0\n99:2:0\n");
}
/** Token cloning grants no additional completion or independent operation capacity. */
static void it_preserves_single_consumer_rights_when_a_native_token_is_copied(void) {
    boundary_schedule("single-consumer", "");
}

/** Copies preserve external operation references while fresh native waits still require new tokens and single-consumer delivery. */
static void it_requires_independent_native_waits_despite_shared_external_operation_references(void) {
    boundary_schedule("shared-operation", "1:1:0\n2:2:0\n2:2:0\n");
}
