/** @file
 * @brief Check/install native readiness under one gate and retain independent registration lifetime.
 * [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * retains synchronous completion; [ASYNC-09](../../../../spec/language/async-and-parallelism.md#async-09)
 * makes obsolete callbacks harmless without borrowing any Ama state.
 */
#include "../internal.h"

/** An empty C owner has no latent key or direction that could disguise ownership. */
static bool empty_wait(const AmaNativeChannelWait *owner) {
    return owner != NULL && owner->state == NULL && owner->callbacks == NULL
        && owner->token.domain == NULL && owner->token.service == 0 && owner->token.generation == 0
        && owner->token.slot == 0 && !owner->sender;
}

/** Clone source rights outside the channel gate. The port pins service lifetime
 * throughout; readiness and installation share one atomic metadata interval.
 * Immediate or already retired/completed sources need no retained channel owner.
 */
static AmaStatus channel_wait_register(AmaNativeChannel *channel, bool sending, size_t required,
    const AmaNativeEventSubmission *source, AmaNativeChannelWait *output) {
    if (!empty_wait(output) || (sending && required == 0)) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaNativeEventAcceptance acceptance = ama_native_event_acceptance(source);
    if (acceptance == AMA_EVENT_ALREADY_COMPLETED || acceptance == AMA_EVENT_TOKEN_RETIRED
        || acceptance == AMA_EVENT_SOURCE_CLOSED) { return ama_status_success(); }
    if (acceptance != AMA_EVENT_ACCEPTING) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaNativeEventSubmission retained = {0};
    AmaStatus status = ama_native_event_submission_clone(source, &retained);
    if (status.code != AMA_OK) { return status; }
    AmaChannelSignals signals = {0};
    ama_channel_lock(channel);
    bool ready = sending ? ama_channel_sender_ready(channel, required)
        : channel->count != 0 || channel->sender_closed || channel->aborted;
    AmaNativeEventSubmission *slot = sending ? &channel->sending.event : &channel->receiving.event;
    if (ready) {
        if (sending) { signals.sending = retained; }
        else { signals.receiving = retained; }
        retained = (AmaNativeEventSubmission){0};
    } else if (slot->state != NULL) { status = ama_status_make(AMA_BUSY, NULL, 0); }
    else if (channel->references == SIZE_MAX) { status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    else {
        channel->references++;
        *slot = retained;
        retained = (AmaNativeEventSubmission){0};
        if (sending) { channel->sending.required_bytes = required; }
        *output = (AmaNativeChannelWait){channel, &ama_channel_wait_callbacks, source->token, sending};
    }
    ama_channel_unlock(channel);
    ama_native_event_submission_drop(&retained);
    ama_channel_publish_signals(&signals);
    return status;
}

/** Each logical sender may have one pending capacity registration, with an exact requested size. */
AmaStatus ama_channel_wait_send(void *state, size_t required, const AmaNativeEventSubmission *source,
    AmaNativeChannelWait *output) {
    return channel_wait_register(state, true, required, source, output);
}
/** Receive readiness is data or terminal closure, independent of application decoding time. */
AmaStatus ama_channel_wait_receive(void *state, const AmaNativeEventSubmission *source, AmaNativeChannelWait *output) {
    return channel_wait_register(state, false, 0, source, output);
}
