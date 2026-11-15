/** @file
 * @brief Exercise atomic check/install, immediate completion and exact capacity predicates.
 * Source token phases are observed directly through creator callbacks rather than inferred from task state.
 */
/** Both condition-before-registration and registration-before-condition retain exactly one signal. */
static void it_retains_receive_readiness_on_both_sides_of_synchronous_registration(void) {
    for (unsigned before = 0; before < 2; before++) {
        AmaNativeChannelSender sender = {0};
        AmaNativeChannelReceiver receiver = {0};
        native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
        AmaNativeEventService *events = create_events(1);
        AmaNativeMessageBytes message = sequence_message(7), output = {0};
        if (before) { channel_reason(ama_native_channel_send(&sender, &message), AMA_CHANNEL_ACCEPTED); }
        AmaNativeChannelWait wait = {0};
        WaitRequest request = {.receiver = &receiver, .output = &wait};
        AmaNativeEventSubmission source = arm_wait(events, &request);
        native_ok(request.status);
        assert(source_ready(&source) == (before != 0));
        assert((wait.state == NULL) == (before != 0));
        if (!before) {
            assert(ama_native_event_acceptance(&source) == AMA_EVENT_ACCEPTING);
            channel_reason(ama_native_channel_send(&sender, &message), AMA_CHANNEL_ACCEPTED);
        }
        assert(source_ready(&source) && ama_native_event_acceptance(&source) == AMA_EVENT_ALREADY_COMPLETED);
        take_signal(events, &source);
        assert(ama_native_event_acceptance(&source) == AMA_EVENT_TOKEN_RETIRED);
        bool retired = true;
        native_ok(ama_native_channel_wait_cancel(&wait, &retired));
        assert(!retired);
        ama_native_channel_wait_drop(&wait);
        channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
        assert(message_sequence(&output) == 7);
        ama_native_message_bytes_drop(&output);
        ama_native_event_submission_drop(&source);
        ama_native_event_service_dispose(&events);
        ama_native_channel_sender_drop(&sender);
        ama_native_channel_receiver_drop(&receiver);
        no_resources();
    }
}

/** Refund must satisfy the requested complete length; free count or a few bytes is not enough. */
static void it_wakes_capacity_only_after_the_exact_requested_length_fits(void) {
    AmaNativeMessageBytes messages[] = {sequence_message(1), sequence_message(2), sequence_message(3)};
    size_t width = messages[0].length;
    AmaNativeMessageField wider_field = field;
    wider_field.maximum_bytes = 200;
    AmaNativeMessageSchema wider_schema = schema;
    wider_schema.fields = &wider_field;
    unsigned char payload[70] = {0};
    AmaNativeMessageValue large_value = {0, {payload, sizeof(payload)}};
    AmaNativeMessageFieldValue large_field = {&large_value, 1};
    AmaNativeMessageBytes large = {0};
    AmaNativeMessageResult encoded = ama_native_message_encode(&wider_schema, &large_field, &provider, &large);
    assert(encoded.status.code == AMA_OK && encoded.reason == AMA_MESSAGE_ACCEPTED);
    assert(large.length > width && large.length <= width * 2);
    size_t required = large.length;
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&wider_schema, 4, width * 3, &provider, &sender, &receiver));
    for (size_t index = 0; index < 3; index++) { channel_reason(ama_native_channel_send(&sender, &messages[index]), AMA_CHANNEL_ACCEPTED); }
    AmaNativeEventService *events = create_events(1);
    AmaNativeChannelWait wait = {0};
    WaitRequest request = {.sender = &sender, .required = required, .output = &wait};
    AmaNativeEventSubmission source = arm_wait(events, &request);
    native_ok(request.status);
    assert(!source_ready(&source));
    AmaNativeMessageBytes output = {0};
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    ama_native_message_bytes_drop(&output);
    assert(!source_ready(&source));
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    ama_native_message_bytes_drop(&output);
    assert(source_ready(&source));
    channel_reason(ama_native_channel_send(&sender, &large), AMA_CHANNEL_ACCEPTED);
    take_signal(events, &source);
    ama_native_channel_wait_drop(&wait);
    ama_native_event_submission_drop(&source);
    ama_native_event_service_dispose(&events);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** An unarmed source cannot install a future publication that would silently lose its signal. */
static void it_rejects_unarmed_sources_and_acks_completed_retired_and_closed_sources(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(1);
    AmaNativeEventSubmission source = {0};
    native_ok(ama_native_event_service_prepare(events, &source));
    AmaNativeChannelWait wait = {0};
    AmaStatus status = ama_native_channel_wait_receive(&receiver, &source, &wait);
    assert(status.code == AMA_INVALID_STATE && wait.state == NULL);
    detach_source(events, &source);
    assert(ama_native_event_acceptance(&source) == AMA_EVENT_TOKEN_RETIRED);
    native_ok(ama_native_channel_wait_receive(&receiver, &source, &wait));
    assert(wait.state == NULL);
    ama_native_event_service_dispose(&events);
    assert(ama_native_event_acceptance(&source) == AMA_EVENT_SOURCE_CLOSED);
    native_ok(ama_native_channel_wait_receive(&receiver, &source, &wait));
    assert(wait.state == NULL);
    ama_native_event_submission_drop(&source);
    assert(ama_native_event_acceptance(&source) == AMA_EVENT_INVALID_SOURCE);
    status = ama_native_channel_wait_receive(&receiver, &source, &wait);
    assert(status.code == AMA_INVALID_STATE);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}
