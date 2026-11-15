/** @file
 * @brief Verify independent count/byte limits, exact ring ordering and capacity release at native take.
 * Received envelopes remain live across slot reuse, independently of application processing.
 */
/** Count saturation preserves the complete sender owner and receipt frees capacity immediately. */
static void it_transfers_whole_fifo_messages_and_releases_capacity_before_processing(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 2, SIZE_MAX, &provider, &sender, &receiver));
    AmaNativeMessageBytes messages[] = {sequence_message(0), sequence_message(1), sequence_message(2)};
    channel_reason(ama_native_channel_send(&sender, &messages[0]), AMA_CHANNEL_ACCEPTED);
    channel_reason(ama_native_channel_send(&sender, &messages[1]), AMA_CHANNEL_ACCEPTED);
    unsigned char *saved = messages[2].data;
    size_t length = messages[2].length, before = atomic_load(&allocation_count);
    channel_reason(ama_native_channel_send(&sender, &messages[2]), AMA_CHANNEL_FULL);
    assert(messages[2].data == saved && messages[2].length == length && atomic_load(&allocation_count) == before);
    AmaNativeMessageBytes first = {0};
    channel_reason(ama_native_channel_take(&receiver, &first), AMA_CHANNEL_RECEIVED);
    channel_reason(ama_native_channel_send(&sender, &messages[2]), AMA_CHANNEL_ACCEPTED);
    assert(message_sequence(&first) == 0);
    ama_native_message_bytes_drop(&first);
    for (uint64_t expected = 1; expected <= 2; expected++) {
        AmaNativeMessageBytes next = {0};
        channel_reason(ama_native_channel_take(&receiver, &next), AMA_CHANNEL_RECEIVED);
        assert(message_sequence(&next) == expected);
        ama_native_message_bytes_drop(&next);
    }
    AmaNativeMessageBytes output = {0};
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_EMPTY);
    native_ok(ama_native_channel_close(&sender));
    native_ok(ama_native_channel_close(&sender));
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_CLOSED);
    AmaNativeMessageBytes closed = sequence_message(3);
    saved = closed.data;
    channel_reason(ama_native_channel_send(&sender, &closed), AMA_CHANNEL_CLOSED);
    assert(closed.data == saved);
    ama_native_message_bytes_drop(&closed);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** Byte saturation is independent of count saturation; one oversize message is rejected whole. */
static void it_enforces_exact_byte_capacity_and_individual_message_size(void) {
    AmaNativeMessageBytes first = sequence_message(1), second = sequence_message(2);
    size_t width = first.length;
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 3, width * 2 - 1, &provider, &sender, &receiver));
    channel_reason(ama_native_channel_send(&sender, &first), AMA_CHANNEL_ACCEPTED);
    unsigned char *saved = second.data;
    channel_reason(ama_native_channel_send(&sender, &second), AMA_CHANNEL_FULL);
    assert(second.data == saved);
    AmaNativeMessageBytes output = {0};
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    channel_reason(ama_native_channel_send(&sender, &second), AMA_CHANNEL_ACCEPTED);
    assert(message_sequence(&output) == 1);
    ama_native_message_bytes_drop(&output);
    ama_native_channel_receiver_drop(&receiver);
    ama_native_channel_sender_drop(&sender);
    no_resources();
    native_ok(ama_native_channel_create(&schema, 3, width - 1, &provider, &sender, &receiver));
    first = sequence_message(0);
    saved = first.data;
    channel_reason(ama_native_channel_send(&sender, &first), AMA_CHANNEL_TOO_LARGE);
    assert(first.data == saved);
    ama_native_message_bytes_drop(&first);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** Every locator wraps repeatedly while values remain distinct and ordered. */
static void it_reuses_ring_slots_without_moving_or_merging_received_owners(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 3, 1024, &provider, &sender, &receiver));
    for (uint64_t sequence = 0; sequence < 20; sequence++) {
        AmaNativeMessageBytes input = sequence_message(sequence), output = {0};
        channel_reason(ama_native_channel_send(&sender, &input), AMA_CHANNEL_ACCEPTED);
        assert(input.data == NULL && input.code.resource == NULL);
        channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
        assert(message_sequence(&output) == sequence);
        ama_native_message_bytes_drop(&output);
    }
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** Different encoded lengths must debit/refund their own bytes, not a per-message constant. */
static void it_refunds_exact_variable_message_lengths_at_take(void) {
    AmaNativeMessageBytes large = sequence_message(0), small = {0};
    AmaNativeMessageValue empty = {0, {NULL, 0}};
    AmaNativeMessageFieldValue value = {&empty, 1};
    AmaNativeMessageResult encoded = ama_native_message_encode(&schema, &value, &provider, &small);
    assert(encoded.status.code == AMA_OK && encoded.reason == AMA_MESSAGE_ACCEPTED && large.length > small.length);
    size_t large_size = large.length, small_size = small.length;
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 3, large_size + small_size, &provider, &sender, &receiver));
    channel_reason(ama_native_channel_send(&sender, &large), AMA_CHANNEL_ACCEPTED);
    channel_reason(ama_native_channel_send(&sender, &small), AMA_CHANNEL_ACCEPTED);
    encoded = ama_native_message_encode(&schema, &value, &provider, &small);
    assert(encoded.status.code == AMA_OK && encoded.reason == AMA_MESSAGE_ACCEPTED);
    channel_reason(ama_native_channel_send(&sender, &small), AMA_CHANNEL_FULL);
    AmaNativeMessageBytes output = {0};
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    assert(output.length == large_size);
    channel_reason(ama_native_channel_send(&sender, &small), AMA_CHANNEL_ACCEPTED);
    ama_native_message_bytes_drop(&output);
    for (size_t index = 0; index < 2; index++) {
        channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
        assert(output.length == small_size);
        ama_native_message_bytes_drop(&output);
    }
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}
