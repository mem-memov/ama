/** @file
 * @brief Bind an armed native readiness source through nested synchronous typed reference visitors.
 * The producer model supplies exact parameter and port-read types; this native
 * companion retains only its channel registration and never an Ama view.
 */
#include "native/events/assembly.h"
/** Stack-only borrow frame; source/parameters expire before the entry returns. */
typedef struct {
    const TEST_WAIT_PARAMETERS *parameters;
    AmaNativeChannelWait *output;
    const AmaNativeEventSubmission *source;
    size_t required;
    AmaStatus status;
} AMA_NATIVE(Borrow);

/** The port visitor may retain native resources through the SDK, never the supplied full view. */
static AmaStatus AMA_NATIVE(port)(const TEST_PORT_READ *port, void *resource) {
    AMA_NATIVE(Borrow) *borrow = resource;
#if TEST_WAIT_SENDING
    return ama_native_channel_wait_send(&port->payload->sender, borrow->required, borrow->source, borrow->output);
#else
    return ama_native_channel_wait_receive(&port->payload->receiver, borrow->source, borrow->output);
#endif
}

/** Event arm invokes this synchronously after publishing armed state and outside its metadata gate. */
static void AMA_NATIVE(start)(const AmaNativeEventSubmission *source, void *resource) {
    AMA_NATIVE(Borrow) *borrow = resource;
    borrow->source = source;
    borrow->status = borrow->parameters->AMA_INPUT(1)->with_read(
        borrow->parameters->AMA_INPUT(1)->storage, AMA_NATIVE(port), borrow);
    borrow->source = NULL;
}

/** Operation state belongs to this image's declared standard module; only native service rights escape. */
static AmaStatus AMA_NATIVE(operation)(const ama_t_3211b272d4ffc58b35668036530071d3838391fee0aa5c4208851404f0010b09 *operation,
    void *resource) {
    AMA_NATIVE(Borrow) *borrow = resource;
    const AmaNativeEventSubmission *source = &operation->payload->submission;
    AmaStatus status = ama_native_event_service_arm(source->state, source, AMA_NATIVE(start), borrow);
    if (status.code == AMA_OK) { status = borrow->status; }
    if (status.code == AMA_OK) {
        void test_wait_observe(const AmaNativeEventSubmission *);
        test_wait_observe(source);
    }
    return status;
}

/** Fresh native ownership is empty; generated fields own all Ama parameters. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {
    *state = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Drop cancels native publication without executing any suspended task body. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    ama_native_channel_wait_drop(&state->wait);
    return ama_status_success();
}
/** Restart acknowledges the previous registration, then arms one fresh declared operation. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const TEST_WAIT_PARAMETERS *parameters,
    TEST_WAIT_RESULTS *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)results;
    (void)services;
    ama_native_channel_wait_drop(&state->wait);
    AMA_NATIVE(Borrow) borrow = {.parameters = parameters, .output = &state->wait};
#if TEST_WAIT_SENDING
    AmaByteView wire;
    *status = ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_view(parameters->AMA_INPUT(2)->payload, &wire);
    if (status->code != AMA_OK) { return; }
    borrow.required = wire.length;
#endif
    *status = parameters->AMA_INPUT(0)->with_read(parameters->AMA_INPUT(0)->storage, AMA_NATIVE(operation), &borrow);
}
