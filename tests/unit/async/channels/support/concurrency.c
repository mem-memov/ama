/** @file
 * @brief Run real single-producer/single-consumer FIFO traffic on separate POSIX threads.
 * Native retry is test-driver scheduling, not an implicit busy loop in an Ama executor.
 */
/** Each worker owns exactly its logical endpoint and native message credit. */
typedef struct { AmaNativeChannelSender sender; AmaNativeChannelReceiver receiver; size_t count; } ChannelWorkers;

/** Publish increasing records; full preserves the same pending owner until native capacity returns. */
static void *channel_producer(void *resource) {
    ChannelWorkers *workers = resource;
    for (size_t sequence = 0; sequence < workers->count; sequence++) {
        AmaNativeMessageBytes message = sequence_message(sequence);
        for (;;) {
            AmaNativeChannelResult result = ama_native_channel_send(&workers->sender, &message);
            assert(result.status.code == AMA_OK);
            if (result.reason == AMA_CHANNEL_ACCEPTED) { break; }
            assert(result.reason == AMA_CHANNEL_FULL && message.data != NULL);
            thrd_yield();
        }
        assert(message.data == NULL && message.code.resource == NULL);
    }
    ama_native_channel_sender_drop(&workers->sender);
    return NULL;
}

/** Drain in exact acceptance order, including data retained after producer retirement. */
static void *channel_consumer(void *resource) {
    ChannelWorkers *workers = resource;
    for (size_t expected = 0; expected < workers->count; expected++) {
        AmaNativeMessageBytes message = {0};
        for (;;) {
            AmaNativeChannelResult result = ama_native_channel_take(&workers->receiver, &message);
            assert(result.status.code == AMA_OK);
            if (result.reason == AMA_CHANNEL_RECEIVED) { break; }
            assert(result.reason == AMA_CHANNEL_EMPTY && message.data == NULL);
            thrd_yield();
        }
        assert(message_sequence(&message) == expected);
        ama_native_message_bytes_drop(&message);
    }
    for (;;) {
        AmaNativeMessageBytes empty = {0};
        AmaNativeChannelResult result = ama_native_channel_take(&workers->receiver, &empty);
        assert(result.status.code == AMA_OK);
        if (result.reason == AMA_CHANNEL_CLOSED) { break; }
        assert(result.reason == AMA_CHANNEL_EMPTY);
        thrd_yield();
    }
    ama_native_channel_receiver_drop(&workers->receiver);
    return NULL;
}

/** Capacity one and a non-power-of-two ring exercise publication/reuse and both limiter paths. */
static void it_transfers_fifo_messages_between_actual_native_producer_and_consumer_threads(void) {
    for (size_t capacity = 1; capacity <= 7; capacity += 6) {
        ChannelWorkers workers = {.count = 2048};
        AmaNativeMessageBytes probe = sequence_message(0);
        size_t byte_limit = probe.length * capacity;
        ama_native_message_bytes_drop(&probe);
        native_ok(ama_native_channel_create(&schema, capacity, byte_limit, &provider, &workers.sender, &workers.receiver));
        pthread_t producer, consumer;
        int result = pthread_create(&producer, NULL, channel_producer, &workers);
        assert(result == 0);
        result = pthread_create(&consumer, NULL, channel_consumer, &workers);
        assert(result == 0);
        void *value = &workers;
        result = pthread_join(producer, &value);
        assert(result == 0 && value == NULL);
        result = pthread_join(consumer, &value);
        assert(result == 0 && value == NULL);
        assert(workers.sender.state == NULL && workers.receiver.state == NULL);
        no_resources();
    }
}

/** Independent endpoint operations share only this exact native barrier and their service. */
typedef struct {
    AmaNativeChannelSender sender;
    AmaNativeChannelReceiver receiver;
    AmaNativeMessageBytes input;
    AmaNativeChannelResult sent;
    AmaStatus aborted;
    _Atomic unsigned arrivals;
} ChannelAbortRace;

/** Both operations start after native publication; no guessed sleep chooses the ordering. */
static void abort_rendezvous(ChannelAbortRace *race) {
    atomic_fetch_add(&race->arrivals, 1);
    while (atomic_load(&race->arrivals) != 2) { thrd_yield(); }
}
/** Either enqueue wins first, or abort preserves this caller's complete input owner. */
static void *send_against_abort(void *argument) {
    ChannelAbortRace *race = argument;
    abort_rendezvous(race);
    race->sent = ama_native_channel_send(&race->sender, &race->input);
    return NULL;
}
/** Detach before release while the opposite sender may be validating outside the gate. */
static void *abort_against_send(void *argument) {
    ChannelAbortRace *race = argument;
    abort_rendezvous(race);
    race->aborted = ama_native_channel_abort(&race->receiver);
    return NULL;
}
/** Both legal linearizations have one owner and exactly one cleanup obligation. */
static void it_linearizes_abort_against_send_without_losing_or_double_releasing_an_owner(void) {
    for (size_t iteration = 0; iteration < 128; iteration++) {
        ChannelAbortRace race = {0};
        atomic_init(&race.arrivals, 0);
        native_ok(ama_native_channel_create(&schema, 1, 256, &provider, &race.sender, &race.receiver));
        race.input = sequence_message(iteration);
        unsigned char *original = race.input.data;
        pthread_t sender, receiver;
        int result = pthread_create(&sender, NULL, send_against_abort, &race);
        assert(result == 0);
        result = pthread_create(&receiver, NULL, abort_against_send, &race);
        assert(result == 0);
        void *value = &race;
        result = pthread_join(sender, &value);
        assert(result == 0 && value == NULL);
        result = pthread_join(receiver, &value);
        assert(result == 0 && value == NULL);
        native_ok(race.aborted);
        assert(race.sent.status.code == AMA_OK);
        if (race.sent.reason == AMA_CHANNEL_ACCEPTED) { assert(race.input.data == NULL); }
        else {
            assert(race.sent.reason == AMA_CHANNEL_ABORTED && race.input.data == original);
            assert(message_sequence(&race.input) == iteration);
        }
        AmaNativeMessageBytes output = {0};
        channel_reason(ama_native_channel_take(&race.receiver, &output), AMA_CHANNEL_ABORTED);
        ama_native_message_bytes_drop(&race.input);
        ama_native_channel_sender_drop(&race.sender);
        ama_native_channel_receiver_drop(&race.receiver);
        no_resources();
    }
}
