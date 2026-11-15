/** @file
 * @brief Detach one-shot readiness rights atomically and notify their native event sources outside all gates.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) and
 * [ASYNC-06](../../../../spec/language/async-and-parallelism.md#async-06)
 * prevent a condition-change gap between checking readiness and installing a wait.
 */
#include <assert.h>
#include "../internal.h"

/** Terminal/impossible sends wake for an ordinary method recheck, never sleep forever. */
bool ama_channel_sender_ready(const AmaNativeChannel *channel, size_t required) {
    return channel->sender_closed || channel->aborted || required > channel->maximum_bytes
        || (channel->count < channel->capacity && required <= channel->maximum_bytes - channel->bytes);
}

/** Caller holds the channel gate. Moving clears publication before any callback can run. */
void ama_channel_collect_signals(AmaNativeChannel *channel, AmaChannelSignals *signals) {
    if (channel->sending.event.state != NULL && ama_channel_sender_ready(channel, channel->sending.required_bytes)) {
        assert(signals->sending.state == NULL);
        signals->sending = channel->sending.event;
        channel->sending.event = (AmaNativeEventSubmission){0};
        channel->sending.required_bytes = 0;
    }
    if (channel->receiving.event.state != NULL && (channel->count != 0 || channel->sender_closed || channel->aborted)) {
        assert(signals->receiving.state == NULL);
        signals->receiving = channel->receiving.event;
        channel->receiving.event = (AmaNativeEventSubmission){0};
    }
}

/** Registration checked armed admission. Full identity cannot become unarmed
 * again; legitimate races only complete, retire or close that original token.
 * No callback is invoked while either native service gate is held.
 */
static void channel_signal(AmaNativeEventSubmission *event) {
    if (event->state == NULL) { return; }
    AmaNativeEventPostResult result = ama_native_event_post(event, 0, NULL);
    assert(result == AMA_EVENT_QUEUED || result == AMA_EVENT_DUPLICATE
        || result == AMA_EVENT_STALE || result == AMA_EVENT_CLOSED);
    (void)result;
    ama_native_event_submission_drop(event);
}

/** Source callbacks queue records only; the appropriate executor performs all Ama work. */
void ama_channel_publish_signals(AmaChannelSignals *signals) {
    channel_signal(&signals->sending);
    channel_signal(&signals->receiving);
}
