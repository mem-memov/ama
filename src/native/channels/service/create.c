/** @file
 * @brief Privately allocate a bounded queue and copied schema before publishing its two logical endpoints.
 * [PAR-04](../../../../spec/language/async-and-parallelism.md#par-04) fixes
 * positive message/byte capacities; [EMBED-06](../../../../spec/implementation/c-library.md#embed-06)
 * retains creator code independently of either Ama context.
 */
#include "../internal.h"

/** No messages exist in a failed construction prefix; release only allocated metadata. */
static void channel_creation_discard(AmaNativeChannel *channel, AmaNativeCodeLease *code) {
    if (channel != NULL) {
        free(channel->cells);
        free(channel->fields);
        free(channel->name);
        free(channel);
    }
    ama_native_code_drop(code);
}

/** Schema/provider views expire after the complete declaration is copied privately. */
AmaStatus ama_native_channel_create(const AmaNativeMessageSchema *schema, size_t maximum_messages,
    size_t maximum_bytes, const AmaNativeCodeProvider *provider, AmaNativeChannelSender *sender,
    AmaNativeChannelReceiver *receiver) {
    if (sender == NULL || receiver == NULL || (void *)sender == (void *)receiver
        || sender->state != NULL || sender->callbacks != NULL || receiver->state != NULL || receiver->callbacks != NULL
        || maximum_messages == 0 || maximum_bytes == 0 || provider == NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaStatus status = ama_native_message_schema_check(schema);
    if (status.code != AMA_OK) { return status; }
    if (maximum_messages > SIZE_MAX / sizeof(AmaNativeMessageBytes)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    AmaNativeCodeLease code = {0};
    status = ama_native_code_acquire(provider, &code);
    if (status.code != AMA_OK) { return status; }
    AmaNativeChannel *channel = malloc(sizeof(*channel));
    if (channel == NULL) { goto allocation_failed; }
    *channel = (AmaNativeChannel){0};
    atomic_init(&channel->locked, false);
    channel->name = malloc(schema->name.length);
    if (channel->name == NULL) { goto allocation_failed; }
    if (schema->field_count != 0) {
        channel->fields = malloc(schema->field_count * sizeof(*channel->fields));
        if (channel->fields == NULL) { goto allocation_failed; }
        memcpy(channel->fields, schema->fields, schema->field_count * sizeof(*channel->fields));
    }
    channel->cells = malloc(maximum_messages * sizeof(*channel->cells));
    if (channel->cells == NULL) { goto allocation_failed; }
    for (size_t index = 0; index < maximum_messages; index++) { channel->cells[index] = (AmaNativeMessageBytes){0}; }
    memcpy(channel->name, schema->name.data, schema->name.length);
    channel->schema = *schema;
    channel->schema.name.data = channel->name;
    channel->schema.fields = channel->fields;
    channel->capacity = maximum_messages;
    channel->maximum_bytes = maximum_bytes;
    channel->references = 2;
    channel->code = code;
    *sender = (AmaNativeChannelSender){channel, &ama_channel_sender_callbacks};
    *receiver = (AmaNativeChannelReceiver){channel, &ama_channel_receiver_callbacks};
    return ama_status_success();
allocation_failed:
    channel_creation_discard(channel, &code);
    return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
}
