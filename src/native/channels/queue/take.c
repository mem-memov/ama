/** @file
 * @brief Transfer the exact FIFO head and release queue count/byte capacity before decoding.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) separates
 * native receipt from application processing and acknowledgment.
 */
#include <assert.h>
#include "../internal.h"

/** A completely empty output receives one independent owner; no view of the ring escapes. */
AmaNativeChannelResult ama_channel_take(void *state, AmaNativeMessageBytes *output) {
    AmaNativeChannel *channel = state;
    if (!ama_channel_empty_owner(output)) { return ama_channel_failure(ama_status_make(AMA_INVALID_STATE, NULL, 0)); }
    AmaChannelSignals signals = {0};
    ama_channel_lock(channel);
    AmaNativeChannelResult result = ama_channel_result(AMA_CHANNEL_RECEIVED);
    if (channel->aborted) { result.reason = AMA_CHANNEL_ABORTED; }
    else if (channel->count == 0) { result.reason = channel->sender_closed ? AMA_CHANNEL_CLOSED : AMA_CHANNEL_EMPTY; }
    else {
        AmaNativeMessageBytes message = channel->cells[channel->head];
        channel->cells[channel->head] = (AmaNativeMessageBytes){0};
        channel->head = ama_channel_next(channel->head, channel->capacity);
        assert(channel->bytes >= message.length);
        channel->bytes -= message.length;
        channel->count--;
        *output = message;
    }
    ama_channel_collect_signals(channel, &signals);
    ama_channel_unlock(channel);
    ama_channel_publish_signals(&signals);
    return result;
}
