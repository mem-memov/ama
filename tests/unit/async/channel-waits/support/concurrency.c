/** @file
 * @brief Race native registration/publication/cancellation using exact barriers and actual source queues.
 * Joins publish results; callbacks queue records only and cannot execute Ama.
 */
/** Each worker owns one logical direction; the source owner serializes prepare/arm/consume. */
typedef struct {
    AmaNativeChannelSender sender;
    AmaNativeChannelReceiver receiver;
    AmaNativeEventService *events;
    AmaNativeEventSubmission source;
    AmaNativeChannelWait wait;
    AmaNativeMessageBytes message;
    AmaStatus registration;
    AmaNativeChannelResult sent;
    _Atomic unsigned arrivals;
} WaitRace;

/** Begin both finite operations after the same native publication frontier. */
static void wait_rendezvous(WaitRace *race) {
    atomic_fetch_add(&race->arrivals, 1);
    while (atomic_load(&race->arrivals) != 2) { thrd_yield(); }
}
/** Arm and install while a producer may enqueue before, during or after the condition check. */
static void *race_register(void *argument) {
    WaitRace *race = argument;
    wait_rendezvous(race);
    WaitRequest request = {.receiver = &race->receiver, .output = &race->wait};
    native_ok(ama_native_event_service_arm(race->events, &race->source, start_channel_wait, &request));
    race->registration = request.status;
    return NULL;
}
/** Only native bytes and source rights are touched by this independent producer. */
static void *race_publish(void *argument) {
    WaitRace *race = argument;
    wait_rendezvous(race);
    race->sent = ama_native_channel_send(&race->sender, &race->message);
    return NULL;
}
/** Both legal check/install orders must leave one retained signal and one queued message. */
static void it_never_loses_receive_readiness_when_registration_races_publication(void) {
    for (size_t iteration = 0; iteration < 128; iteration++) {
        WaitRace race = {0};
        atomic_init(&race.arrivals, 0);
        native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &race.sender, &race.receiver));
        race.events = create_events(1);
        native_ok(ama_native_event_service_prepare(race.events, &race.source));
        race.message = sequence_message(iteration);
        pthread_t registration, publication;
        int result = pthread_create(&registration, NULL, race_register, &race);
        assert(result == 0);
        result = pthread_create(&publication, NULL, race_publish, &race);
        assert(result == 0);
        void *value = &race;
        result = pthread_join(registration, &value);
        assert(result == 0 && value == NULL);
        result = pthread_join(publication, &value);
        assert(result == 0 && value == NULL);
        native_ok(race.registration);
        channel_reason(race.sent, AMA_CHANNEL_ACCEPTED);
        assert(source_ready(&race.source));
        take_signal(race.events, &race.source);
        AmaNativeMessageBytes output = {0};
        channel_reason(ama_native_channel_take(&race.receiver, &output), AMA_CHANNEL_RECEIVED);
        assert(message_sequence(&output) == iteration);
        ama_native_message_bytes_drop(&output);
        ama_native_channel_wait_drop(&race.wait);
        ama_native_event_submission_drop(&race.source);
        ama_native_event_service_dispose(&race.events);
        ama_native_channel_sender_drop(&race.sender);
        ama_native_channel_receiver_drop(&race.receiver);
        no_resources();
    }
}

/** Close on another endpoint wakes a genuinely blocked native host owner, with no timing oracle. */
static void *close_sender_worker(void *argument) {
    AmaNativeChannelSender *sender = argument;
    ama_native_channel_sender_drop(sender);
    return NULL;
}
/** Both close-before-wait and wait-before-close are covered by the source's retained notification. */
static void it_wakes_a_native_host_wait_when_the_other_endpoint_closes(void) {
    AmaNativeChannelSender sender = {0};
    AmaNativeChannelReceiver receiver = {0};
    native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &sender, &receiver));
    AmaNativeEventService *events = create_events(1);
    AmaNativeChannelWait wait = {0};
    WaitRequest request = {.receiver = &receiver, .output = &wait};
    AmaNativeEventSubmission source = arm_wait(events, &request);
    native_ok(request.status);
    pthread_t closer;
    int result = pthread_create(&closer, NULL, close_sender_worker, &sender);
    assert(result == 0);
    bool ready = false;
    native_ok(ama_native_event_service_wait(events, &ready));
    assert(ready);
    void *value = &sender;
    result = pthread_join(closer, &value);
    assert(result == 0 && value == NULL);
    take_signal(events, &source);
    AmaNativeMessageBytes empty = {0};
    channel_reason(ama_native_channel_take(&receiver, &empty), AMA_CHANNEL_CLOSED);
    ama_native_channel_wait_drop(&wait);
    ama_native_event_submission_drop(&source);
    ama_native_event_service_dispose(&events);
    ama_native_channel_receiver_drop(&receiver);
    no_resources();
}

/** Visible delivery invalidation wins independently of whether publication already selected its source right. */
static void it_retires_delivery_when_cancellation_races_an_already_selected_publication(void) {
    for (size_t iteration = 0; iteration < 128; iteration++) {
        WaitRace race = {0};
        atomic_init(&race.arrivals, 0);
        native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &race.sender, &race.receiver));
        race.events = create_events(1);
        WaitRequest request = {.receiver = &race.receiver, .output = &race.wait};
        race.source = arm_wait(race.events, &request);
        native_ok(request.status);
        race.message = sequence_message(iteration);
        pthread_t publication;
        int result = pthread_create(&publication, NULL, race_publish, &race);
        assert(result == 0);
        wait_rendezvous(&race);
        detach_source(race.events, &race.source);
        bool retired = false;
        native_ok(ama_native_channel_wait_cancel(&race.wait, &retired));
        void *value = &race;
        result = pthread_join(publication, &value);
        assert(result == 0 && value == NULL);
        channel_reason(race.sent, AMA_CHANNEL_ACCEPTED);
        assert(ama_native_event_acceptance(&race.source) == AMA_EVENT_TOKEN_RETIRED);
        AmaNativeEventCompletion completion = {0};
        bool available = true;
        native_ok(ama_native_event_service_take(race.events, &completion, &available));
        assert(!available);
        AmaNativeMessageBytes output = {0};
        channel_reason(ama_native_channel_take(&race.receiver, &output), AMA_CHANNEL_RECEIVED);
        assert(message_sequence(&output) == iteration);
        ama_native_message_bytes_drop(&output);
        ama_native_channel_wait_drop(&race.wait);
        ama_native_event_submission_drop(&race.source);
        ama_native_event_service_dispose(&race.events);
        ama_native_channel_sender_drop(&race.sender);
        ama_native_channel_receiver_drop(&race.receiver);
        no_resources();
    }
}
