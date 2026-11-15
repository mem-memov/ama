/** @file
 * @brief Reject bad protocol data and malformed native owner shapes without consuming input or FIFO state.
 * Schema copying is checked by mutating the original declaration after creation.
 */
/** Captured schema storage is independent; invalid wire and nonempty outputs leave all owners intact. */
static void it_copies_schema_and_preserves_rejected_inputs_and_receiver_outputs(void) {
    unsigned char name[sizeof(protocol_name) - 1];
    memcpy(name, protocol_name, sizeof(name));
    AmaNativeMessageField local_field = field;
    AmaNativeMessageSchema local = schema;
    local.name = (AmaByteView){name, sizeof(name)};
    local.fields = &local_field;
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&local, 2, 1024, &provider, &sender, &receiver));
    memset(name, 255, sizeof(name));
    local_field.maximum_bytes = 0;
    local.version++;
    AmaNativeMessageBytes input = sequence_message(1), output = {0};
    unsigned char *saved = input.data;
    input.data[0] = 'X';
    size_t before = atomic_load(&allocation_count);
    AmaNativeChannelResult result = ama_native_channel_send(&sender, &input);
    channel_reason(result, AMA_CHANNEL_SCHEMA_REJECTED);
    assert(result.message_reason == AMA_MESSAGE_UNSUPPORTED_FORMAT && input.data == saved);
    assert(atomic_load(&allocation_count) == before);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_EMPTY);
    input.data[0] = 'A';
    AmaNativeCodeLease code = input.code;
    input.code.release = NULL;
    result = ama_native_channel_send(&sender, &input);
    assert(result.status.code == AMA_INVALID_STATE && input.data == saved);
    input.code = code;
    channel_reason(ama_native_channel_send(&sender, &input), AMA_CHANNEL_ACCEPTED);
    output = sequence_message(9);
    saved = output.data;
    result = ama_native_channel_take(&receiver, &output);
    assert(result.status.code == AMA_INVALID_STATE && output.data == saved);
    ama_native_message_bytes_drop(&output);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    assert(message_sequence(&output) == 1);
    ama_native_message_bytes_drop(&output);
    result = ama_native_channel_send(&sender, &input);
    channel_reason(result, AMA_CHANNEL_SCHEMA_REJECTED);
    assert(result.message_reason == AMA_MESSAGE_MALFORMED && input.data == NULL);
    AmaNativeChannelSender empty_sender = {0};
    AmaNativeChannelReceiver empty_receiver = {0};
    result = ama_native_channel_send(&empty_sender, &input);
    assert(result.status.code == AMA_INVALID_STATE);
    result = ama_native_channel_take(&empty_receiver, &output);
    assert(result.status.code == AMA_INVALID_STATE);
    assert(ama_native_channel_close(&empty_sender).code == AMA_INVALID_STATE);
    assert(ama_native_channel_abort(&empty_receiver).code == AMA_INVALID_STATE);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** Independent channels preserve only their own FIFO; consumption need not follow global send order. */
static void it_keeps_independent_channel_order_and_ownership_separate(void) {
    AmaNativeChannelSender senders[2] = {0};
    AmaNativeChannelReceiver receivers[2] = {0};
    for (size_t index = 0; index < 2; index++) {
        native_ok(ama_native_channel_create(&schema, 2, 1024, &provider, &senders[index], &receivers[index]));
    }
    AmaNativeMessageBytes one = sequence_message(1), two = sequence_message(2), three = sequence_message(3);
    channel_reason(ama_native_channel_send(&senders[0], &one), AMA_CHANNEL_ACCEPTED);
    channel_reason(ama_native_channel_send(&senders[1], &two), AMA_CHANNEL_ACCEPTED);
    channel_reason(ama_native_channel_send(&senders[0], &three), AMA_CHANNEL_ACCEPTED);
    const size_t channel_order[] = {1, 0, 0};
    const uint64_t message_order[] = {2, 1, 3};
    for (size_t index = 0; index < 3; index++) {
        AmaNativeMessageBytes output = {0};
        channel_reason(ama_native_channel_take(&receivers[channel_order[index]], &output), AMA_CHANNEL_RECEIVED);
        assert(message_sequence(&output) == message_order[index]);
        ama_native_message_bytes_drop(&output);
    }
    for (size_t index = 0; index < 2; index++) {
        ama_native_channel_sender_drop(&senders[index]);
        ama_native_channel_receiver_drop(&receivers[index]);
    }
    no_resources();
}
