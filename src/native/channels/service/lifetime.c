/** @file
 * @brief Retire native channel storage only after both independently owned endpoints disappear.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) lets
 * accepted messages survive sender disposal and makes receiver disposal abort delivery.
 */
#include <assert.h>
#include "../internal.h"

/** Port drop already performed close/abort; final cleanup invokes no callback under the gate. */
void ama_channel_release(AmaNativeChannel *channel) {
    ama_channel_lock(channel);
    assert(channel->references > 0);
    channel->references--;
    bool final = channel->references == 0;
    ama_channel_unlock(channel);
    if (!final) { return; }
    assert(channel->aborted && channel->sender_closed && channel->count == 0 && channel->bytes == 0);
    AmaNativeCodeLease code = channel->code;
    free(channel->cells);
    free(channel->fields);
    free(channel->name);
    free(channel);
    ama_native_code_drop(&code);
}
