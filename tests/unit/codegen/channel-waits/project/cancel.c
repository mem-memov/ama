/** @file
 * @brief Cancel an exact native wait through a synchronous declared Ama reference.
 * Delivery invalidation precedes this call in the cooperative task body.
 */
/** Borrowed target state remains immutable; cancellation changes its independent native service only. */
static AmaStatus AMA_NATIVE(cancel)(const TEST_WAIT_READ *target, void *unused) {
    (void)unused;
    bool retired = false;
    return ama_native_channel_wait_cancel(&target->payload->wait, &retired);
}
/** This command owns no resource or hidden Ama relationship. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    *state = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Generated ownership owns all declared parameters; no native cleanup is needed. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** Expose exact cancellation while retaining handle storage until normal owning-module disposal. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const TEST_WAIT_PARAMETERS *parameters,
    TEST_WAIT_RESULTS *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state;
    (void)results;
    (void)services;
    *status = parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage, AMA_NATIVE(cancel), NULL);
}
