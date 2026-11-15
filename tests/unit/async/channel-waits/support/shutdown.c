/** @file
 * @brief Wake readiness on terminal endpoint transitions and retire native sources after context-like shutdown.
 * Registration handles keep creator code/storage alive independently of both ports.
 */
/** Sender close wakes an empty receiver; receiver abort wakes a capacity-blocked sender. */
static void it_wakes_both_directions_on_their_terminal_endpoint_transitions(void) {
    for (unsigned sending = 0; sending < 2; sending++) {
        AmaNativeChannelSender sender = {0};
        AmaNativeChannelReceiver receiver = {0};
        native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
        AmaNativeEventService *events = create_events(1);
        size_t required = 1;
        if (sending) {
            AmaNativeMessageBytes message = sequence_message(1);
            required = message.length;
            channel_reason(ama_native_channel_send(&sender, &message), AMA_CHANNEL_ACCEPTED);
        }
        AmaNativeChannelWait wait = {0};
        WaitRequest request = {.sender = sending ? &sender : NULL,
            .receiver = sending ? NULL : &receiver, .required = required, .output = &wait};
        AmaNativeEventSubmission source = arm_wait(events, &request);
        native_ok(request.status);
        assert(!source_ready(&source) && wait.state != NULL);
        if (sending) { native_ok(ama_native_channel_abort(&receiver)); }
        else { native_ok(ama_native_channel_close(&sender)); }
        assert(source_ready(&source));
        take_signal(events, &source);
        ama_native_channel_sender_drop(&sender);
        ama_native_channel_receiver_drop(&receiver);
        assert(atomic_load(&code_credits) == 2);
        bool retired = true;
        native_ok(ama_native_channel_wait_cancel(&wait, &retired));
        assert(!retired);
        ama_native_channel_wait_drop(&wait);
        ama_native_event_submission_drop(&source);
        ama_native_event_service_dispose(&events);
        no_resources();
    }
}

/** Closed source acknowledgment cannot leak a callback or pin a former context indefinitely. */
static void it_retires_closed_event_sources_and_channel_storage_after_both_ports_disappear(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(1);
    AmaNativeChannelWait wait = {0};
    WaitRequest request = {.receiver = &receiver, .output = &wait};
    AmaNativeEventSubmission source = arm_wait(events, &request);
    native_ok(request.status);
    ama_native_event_service_dispose(&events);
    ama_native_event_submission_drop(&source);
    assert(atomic_load(&code_credits) == 2);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    assert(atomic_load(&code_credits) == 1);
    assert(atomic_load(&live_allocations) == native_history + 4);
    bool retired = true;
    native_ok(ama_native_channel_wait_cancel(&wait, &retired));
    assert(!retired);
    ama_native_channel_wait_drop(&wait);
    no_resources();
}

/** An individually impossible send wakes for its ordinary size result rather than retaining a doomed wait. */
static void it_signals_oversize_and_already_completed_requests_without_redundant_owners(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 1, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(1);
    AmaNativeChannelWait wait = {0};
    WaitRequest request = {.sender = &sender, .required = 2, .output = &wait};
    AmaNativeEventSubmission source = arm_wait(events, &request);
    native_ok(request.status);
    assert(wait.state == NULL && source_ready(&source));
    native_ok(ama_native_channel_wait_receive(&receiver, &source, &wait));
    assert(wait.state == NULL);
    take_signal(events, &source);
    ama_native_event_submission_drop(&source);
    ama_native_event_service_dispose(&events);
    ama_native_channel_sender_drop(&sender);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}
