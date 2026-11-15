/** @file
 * @brief Exercise exact native reference-count exhaustion and reject malformed registration shapes before publication.
 * Private counter writes are explicit single-threaded fault injection; all effects use public SDK calls.
 */
#include "native/channels/internal.h"
#include "native/events/internal.h"

/** Neither source-reference nor channel-reference overflow may publish a partially retained wait. */
static void it_preserves_wait_outputs_on_reference_exhaustion_and_invalid_source_shapes(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(1);
    AmaNativeEventSubmission source = arm_idle(events);
    AmaNativeChannelWait wait = {0};
    AmaNativeChannel *channel = receiver.state;
    size_t references = channel->references, source_references = events->references;
    size_t allocations = atomic_load(&allocation_count);
    channel->references = SIZE_MAX;
    AmaStatus status = ama_native_channel_wait_receive(&receiver, &source, &wait);
    assert(status.code == AMA_LIMIT_EXCEEDED && wait.state == NULL && events->references == source_references);
    channel->references = references;
    events->references = SIZE_MAX;
    status = ama_native_channel_wait_receive(&receiver, &source, &wait);
    assert(status.code == AMA_LIMIT_EXCEEDED && wait.state == NULL && channel->references == references);
    events->references = source_references;
    status = ama_native_channel_wait_send(&sender, 0, &source, &wait);
    assert(status.code == AMA_INVALID_STATE && wait.state == NULL);
    AmaNativeEventCallbacks malformed = *source.callbacks;
    malformed.acceptance = NULL;
    AmaNativeEventSubmission borrowed = source;
    borrowed.callbacks = &malformed;
    status = ama_native_channel_wait_receive(&receiver, &borrowed, &wait);
    assert(status.code == AMA_INVALID_STATE && wait.state == NULL);
    malformed = *source.callbacks;
    malformed.post = NULL;
    status = ama_native_channel_wait_receive(&receiver, &borrowed, &wait);
    assert(status.code == AMA_INVALID_STATE && wait.state == NULL);
    native_ok(ama_native_channel_wait_receive(&receiver, &source, &wait));
    assert(wait.state != NULL && atomic_load(&allocation_count) == allocations);
    void *original = wait.state;
    status = ama_native_channel_wait_receive(&receiver, &source, &wait);
    assert(status.code == AMA_INVALID_STATE && wait.state == original);
    detach_source(events, &source);
    ama_native_channel_wait_drop(&wait);
    ama_native_event_submission_drop(&source);
    ama_native_event_service_dispose(&events);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}
