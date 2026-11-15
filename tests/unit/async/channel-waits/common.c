/** @file
 * @brief Compose real channel/event ownership with exact synchronous arm and one-shot registration helpers.
 * Native identity history is explicitly distinct from live allocation/code ownership.
 */
#ifndef AMA_CHANNEL_WAIT_TEST_COMMON_C
#define AMA_CHANNEL_WAIT_TEST_COMMON_C
#include "../channels/common.c"

/** Reserve the single unrecycled native image receipt before exact live-resource scenarios. */
static void prepare_identity_history(void) {
    AmaNativeEventService *events = NULL;
    native_ok(ama_native_event_service_create(1, NULL, &events));
    ama_native_event_service_dispose(&events);
    native_history = atomic_load(&live_allocations);
    assert(native_history == 1);
    no_resources();
}

/** A source service owns an independent creator credit until its final native endpoint retires. */
static AmaNativeEventService *create_events(size_t capacity) {
    AmaNativeCodeLease code = {0};
    native_ok(ama_native_code_acquire(&provider, &code));
    AmaNativeEventKeepalive keepalive = {code.resource, code.release};
    code = (AmaNativeCodeLease){0};
    AmaNativeEventService *events = NULL;
    native_ok(ama_native_event_service_create(capacity, &keepalive, &events));
    assert(keepalive.resource == NULL && keepalive.release == NULL);
    return events;
}

/** Stack-only arguments to the synchronous event arm callback, never retained by a service. */
typedef struct {
    const AmaNativeChannelSender *sender;
    const AmaNativeChannelReceiver *receiver;
    size_t required;
    AmaNativeChannelWait *output;
    AmaStatus status;
} WaitRequest;

/** The source publishes armed state before invoking this provider; registration may signal immediately. */
static void start_channel_wait(const AmaNativeEventSubmission *source, void *resource) {
    WaitRequest *request = resource;
    request->status = request->sender != NULL
        ? ama_native_channel_wait_send(request->sender, request->required, source, request->output)
        : ama_native_channel_wait_receive(request->receiver, source, request->output);
}

/** Prepare and arm one source with its actual registration callback; expose callback status independently. */
static AmaNativeEventSubmission arm_wait(AmaNativeEventService *events, WaitRequest *request) {
    AmaNativeEventSubmission source = {0};
    native_ok(ama_native_event_service_prepare(events, &source));
    assert(ama_native_event_acceptance(&source) == AMA_EVENT_NOT_ARMED);
    native_ok(ama_native_event_service_arm(events, &source, start_channel_wait, request));
    return source;
}

/** Leave one armed source available for explicit provider-admission boundary checks. */
static void idle_provider(const AmaNativeEventSubmission *source, void *unused) {
    (void)source;
    (void)unused;
}
/** Arm through the actual source owner without installing a channel subscription yet. */
static AmaNativeEventSubmission arm_idle(AmaNativeEventService *events) {
    AmaNativeEventSubmission source = {0};
    native_ok(ama_native_event_service_prepare(events, &source));
    native_ok(ama_native_event_service_arm(events, &source, idle_provider, NULL));
    return source;
}

/** Observe retained native readiness without executing or resolving any Ama storage. */
static bool source_ready(const AmaNativeEventSubmission *source) {
    bool ready = false;
    native_ok(ama_native_event_ready(source, &ready));
    return ready;
}

/** Import precisely this native token, with code zero and no data, then retire the packet owner. */
static void take_signal(AmaNativeEventService *events, const AmaNativeEventSubmission *source) {
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    native_ok(ama_native_event_service_take(events, &completion, &available));
    assert(available && completion.code == 0 && completion.buffer.length == 0);
    assert(ama_native_event_token_equal(completion.token, source->token));
    ama_native_event_completion_drop(&completion);
}

/** Invalidate native delivery before channel publication cancellation, matching the task ownership contract. */
static void detach_source(AmaNativeEventService *events, const AmaNativeEventSubmission *source) {
    bool retired = false;
    native_ok(ama_native_event_service_detach(events, source->token, &retired));
}
#endif
