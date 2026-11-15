/** @file
 * @brief Distinguish producer close/drain from receiver abort without releasing messages under a gate.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) preserves
 * accepted messages after sender close and explicitly withdraws that promise on abort.
 */
#include "../internal.h"

/** Closing is a metadata transition only; accepted FIFO cells remain owned. */
AmaStatus ama_channel_close(void *state) {
    AmaNativeChannel *channel = state;
    AmaChannelSignals signals = {0};
    ama_channel_lock(channel);
    channel->sender_closed = true;
    ama_channel_collect_signals(channel, &signals);
    ama_channel_unlock(channel);
    ama_channel_publish_signals(&signals);
    return ama_status_success();
}

/** Detach the complete ring range before releases. Aborted admission prevents
 * any producer from editing those cells after unlock, while the receiver port
 * keeps service storage alive until every detached owner has retired.
 */
AmaStatus ama_channel_abort(void *state) {
    AmaNativeChannel *channel = state;
    AmaChannelSignals signals = {0};
    ama_channel_lock(channel);
    size_t head = channel->head, count = channel->count, capacity = channel->capacity;
    channel->aborted = true;
    channel->count = channel->bytes = channel->head = channel->tail = 0;
    ama_channel_collect_signals(channel, &signals);
    ama_channel_unlock(channel);
    for (size_t index = 0; index < count; index++) {
        ama_native_message_bytes_drop(&channel->cells[head]);
        head = ama_channel_next(head, capacity);
    }
    ama_channel_publish_signals(&signals);
    return ama_status_success();
}
