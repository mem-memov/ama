/** @file
 * @brief Retire exact channel/event registrations without consuming queued data or touching a replacement key.
 * Native event delivery is invalidated before publication cancellation, matching task cancellation ordering.
 */
/** Conflicting waits preserve the old registration; a late old cancel cannot erase a reused-slot successor. */
static void it_cancels_exact_keys_and_preserves_a_replacement_on_the_same_event_slot(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(2);
    AmaNativeChannelWait old = {0}, conflict = {0}, replacement = {0};
    WaitRequest first = {.receiver = &receiver, .output = &old};
    AmaNativeEventSubmission source = arm_wait(events, &first);
    native_ok(first.status);
    WaitRequest second = {.receiver = &receiver, .output = &conflict};
    AmaNativeEventSubmission competing = arm_wait(events, &second);
    assert(second.status.code == AMA_BUSY && conflict.state == NULL && !source_ready(&source));
    detach_source(events, &competing);
    ama_native_event_submission_drop(&competing);
    detach_source(events, &source);
    bool retired = false;
    native_ok(ama_native_channel_wait_cancel(&old, &retired));
    assert(retired);
    native_ok(ama_native_channel_wait_cancel(&old, &retired));
    assert(!retired && old.state != NULL);
    WaitRequest third = {.receiver = &receiver, .output = &replacement};
    AmaNativeEventSubmission fresh = arm_wait(events, &third);
    native_ok(third.status);
    assert(fresh.token.slot == source.token.slot && fresh.token.generation != source.token.generation);
    retired = true;
    native_ok(ama_native_channel_wait_cancel(&old, &retired));
    assert(!retired && !source_ready(&fresh));
    AmaNativeMessageBytes message = sequence_message(9), output = {0};
    channel_reason(ama_native_channel_send(&sender, &message), AMA_CHANNEL_ACCEPTED);
    assert(source_ready(&fresh));
    take_signal(events, &fresh);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    assert(message_sequence(&output) == 9);
    ama_native_message_bytes_drop(&output);
    ama_native_channel_wait_drop(&old);
    ama_native_channel_wait_drop(&replacement);
    ama_native_event_submission_drop(&source);
    ama_native_event_submission_drop(&fresh);
    ama_native_event_service_dispose(&events);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** Cancelled readiness does not abort the channel or discard a later accepted application value. */
static void it_withdraws_publication_without_discarding_channel_data(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(1);
    AmaNativeChannelWait wait = {0};
    WaitRequest request = {.receiver = &receiver, .output = &wait};
    AmaNativeEventSubmission source = arm_wait(events, &request);
    native_ok(request.status);
    detach_source(events, &source);
    bool retired = false;
    native_ok(ama_native_channel_wait_cancel(&wait, &retired));
    assert(retired);
    AmaNativeMessageBytes message = sequence_message(42), output = {0};
    channel_reason(ama_native_channel_send(&sender, &message), AMA_CHANNEL_ACCEPTED);
    AmaNativeEventCompletion completion = {0};
    bool available = true;
    native_ok(ama_native_event_service_take(events, &completion, &available));
    assert(!available && completion.buffer.owner == NULL);
    channel_reason(ama_native_channel_take(&receiver, &output), AMA_CHANNEL_RECEIVED);
    assert(message_sequence(&output) == 42);
    ama_native_message_bytes_drop(&output);
    ama_native_channel_wait_drop(&wait);
    ama_native_channel_wait_drop(&wait);
    retired = true;
    native_ok(ama_native_channel_wait_cancel(&wait, &retired));
    assert(!retired);
    ama_native_event_submission_drop(&source);
    ama_native_event_service_dispose(&events);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}
