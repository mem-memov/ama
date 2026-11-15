/** @file
 * @brief Verify sender-close draining, receiver-abort cleanup and independent taken-message lifetime.
 * Every port/message/code credit is explicitly retired, including repeated empty disposal.
 */
/** A sender can disappear before receive; a taken owner can outlive the entire service. */
static void it_drains_after_sender_disposal_and_retains_taken_data_after_service_retirement(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 2, 1024, &provider, &sender, &receiver));
    AmaNativeMessageBytes first = sequence_message(7), second = sequence_message(8), output = {0};
    channel_reason(ama_native_channel_send(&sender, &first), AMA_CHANNEL_ACCEPTED);
    channel_reason(ama_native_channel_send(&sender, &second), AMA_CHANNEL_ACCEPTED);
    ama_native_channel_sender_drop(&sender);
    assert(sender.state == NULL && sender.callbacks == NULL);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    assert(message_sequence(&output) == 7);
    ama_native_message_bytes_drop(&output);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    AmaNativeMessageBytes empty = {0};
    channel_reason(ama_native_channel_take(&receiver, &empty), AMA_CHANNEL_CLOSED);
    ama_native_channel_receiver_drop(&receiver);
    assert(atomic_load(&live_allocations) == 1 && atomic_load(&code_credits) == 1);
    assert(message_sequence(&output) == 8);
    ama_native_message_bytes_drop(&output);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    ama_native_channel_sender_drop(NULL);
    ama_native_channel_receiver_drop(NULL);
    no_resources();
}

/** Abort consumes only queued owners; rejected new input remains the sender's obligation. */
static void it_aborts_delivery_and_preserves_rejected_sender_data(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 3, 1024, &provider, &sender, &receiver));
    for (uint64_t index = 0; index < 3; index++) {
        AmaNativeMessageBytes input = sequence_message(index);
        channel_reason(ama_native_channel_send(&sender, &input), AMA_CHANNEL_ACCEPTED);
    }
    native_ok(ama_native_channel_abort(&receiver));
    native_ok(ama_native_channel_abort(&receiver));
    assert(atomic_load(&code_credits) == 1);
    AmaNativeMessageBytes input = sequence_message(9), output = {0};
    unsigned char *saved = input.data;
    channel_reason(ama_native_channel_send(&sender, &input), AMA_CHANNEL_ABORTED);
    assert(input.data == saved);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_ABORTED);
    ama_native_message_bytes_drop(&input);
    ama_native_channel_receiver_drop(&receiver);
    ama_native_channel_sender_drop(&sender);
    no_resources();
}
