/** @file
 * @brief Drive cooperative channel actors, premature host resume, explicit cancellation and isolated failures.
 * Host observations own native event endpoints only; no Ama view/handle is retained by a callback.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "native/channels/channel.h"
static size_t live_allocations, identity_history;
static AmaNativeChannelSender host_sender;
static AmaNativeChannelReceiver host_receiver;
static AmaNativeEventSubmission observed[2];
static bool current_sender;
static unsigned scenario;
/** Original libc allocator behind direct project allocation accounting. */
void *__real_malloc(size_t);
/** Original zeroing allocator, including compiler-folded initialization. */
void *__real_calloc(size_t, size_t);
/** Original free, called after consuming one actual allocation credit. */
void __real_free(void *);
/** Count direct runtime/native allocations without attributing libc internals to Ama. */
void *__wrap_malloc(size_t size) {
    void *memory = __real_malloc(size);
    if (memory != NULL) { live_allocations++; }
    return memory;
}
/** Optimized native service construction may use calloc rather than spelled malloc. */
void *__wrap_calloc(size_t count, size_t size) {
    void *memory = __real_calloc(count, size);
    if (memory != NULL) { live_allocations++; }
    return memory;
}
/** No live native/Ama allocation may be released twice or after its accounting ended. */
void __wrap_free(void *memory) {
    if (memory != NULL) {
        if (live_allocations == 0) { abort(); }
        live_allocations--;
    }
    __real_free(memory);
}
/** Native factory moves the one logical sender into the matching context's owned payload. */
void test_channel_claim_sender(AmaNativeChannelSender *output) {
    if (current_sender) { *output = host_sender; host_sender = (AmaNativeChannelSender){0}; }
}
/** The other context receives the independent receiver right. */
void test_channel_claim_receiver(AmaNativeChannelReceiver *output) {
    if (!current_sender) { *output = host_receiver; host_receiver = (AmaNativeChannelReceiver){0}; }
}
/** Role is explicit host setup data, never an implicit Ama current-instance selector. */
bool test_channel_role(void) { return current_sender; }
/** Controlled technical failure belongs to exactly one selected actor/context. */
bool test_channel_should_fail(void) { return (scenario == 1 && current_sender) || (scenario == 2 && !current_sender); }
/** Cancellation is observed by the scheduler at a cooperative checkpoint. */
bool test_channel_should_cancel(void) { return scenario == 3 && current_sender; }
/** Expected terminal task outcome is distinct from ordinary channel failure data. */
unsigned test_channel_expected_outcome(void) { return scenario == 3 ? 1u : 2u; }
/** Receiver abort is an ordinary send failure; it does not technically fail the sender context. */
bool test_channel_expected_failure(void) { return scenario == 2; }
/** Retain only independently owned native rights before the typed reference visitor ends. */
void test_wait_observe(const AmaNativeEventSubmission *source) {
    unsigned index = current_sender ? 1 : 0;
    ama_native_event_submission_drop(&observed[index]);
    AmaStatus status = ama_native_event_submission_clone(source, &observed[index]);
    if (status.code != AMA_OK) { abort(); }
}
#include "project.c"
/** Preserve executable assertions in release-generated consumers with useful invariant diagnostics. */
static void require_at(bool value, const char *expression, int line) {
    if (!value) { fprintf(stderr, "Channel wait scenario %u:%d: %s (live=%zu)\n", scenario, line, expression, live_allocations); abort(); }
    assert(value);
}
#define require(value) require_at((value), #value, __LINE__)

/** Capture exact native readiness without resolving any context-owned field. */
static bool native_ready(unsigned index) {
    bool ready = false;
    require(ama_native_event_ready(&observed[index], &ready).code == AMA_OK);
    return ready;
}
/** Completed contexts dispose successfully; a technical failure retains its diagnosed status. */
static void dispose_actor(AmaProjectHost *host, bool failed) {
    AmaStatus status = ama_project_dispose(host);
    require(status.code == (failed ? AMA_NATIVE_FAILURE : AMA_OK));
}

/** Run ordinary, sender-failure, receiver-failure, cancellation and receive-first arrangements. */
int main(void) {
    AmaNativeEventService *bootstrap = NULL;
    require(ama_native_event_service_create(1, NULL, &bootstrap).code == AMA_OK);
    ama_native_event_service_dispose(&bootstrap);
    identity_history = live_allocations;
    require(identity_history == 1);
    for (scenario = 0; scenario < 5; scenario++) {
        const AmaNativeCodeProvider resident = {0};
        require(ama_native_channel_create(&test_message_schema, 1, 512, &resident,
            &host_sender, &host_receiver).code == AMA_OK);
        AmaProjectHost producer = {0}, receiver = {0};
        AmaHostResult result;
        if (scenario == 4) {
            current_sender = false;
            result = ama_project_execute(&receiver, AMA_ACCESS_ENTRY);
            require(result.outcome == AMA_HOST_YIELDED && !native_ready(0));
            result = ama_project_execute(&receiver, AMA_ACCESS_RESUME);
            require(result.outcome == AMA_HOST_YIELDED && !native_ready(0));
        }
        current_sender = true;
        result = ama_project_execute(&producer, AMA_ACCESS_ENTRY);
        if (scenario == 1) {
            require(result.outcome == AMA_HOST_FAILED && result.status.code == AMA_NATIVE_FAILURE);
            require(ama_native_event_acceptance(&observed[1]) == AMA_EVENT_SOURCE_CLOSED);
            current_sender = false;
            result = ama_project_execute(&receiver, AMA_ACCESS_ENTRY);
            require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
        } else if (scenario == 3) {
            require(result.outcome == AMA_HOST_YIELDED && !native_ready(1));
            result = ama_project_execute(&producer, AMA_ACCESS_RESUME);
            require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
            require(ama_native_event_acceptance(&observed[1]) == AMA_EVENT_TOKEN_RETIRED);
            current_sender = false;
            result = ama_project_execute(&receiver, AMA_ACCESS_ENTRY);
            require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
        } else {
            require(result.outcome == AMA_HOST_YIELDED && !native_ready(1));
            result = ama_project_execute(&producer, AMA_ACCESS_RESUME);
            require(result.outcome == AMA_HOST_YIELDED && !native_ready(1));
            current_sender = false;
            result = ama_project_execute(&receiver, scenario == 4 ? AMA_ACCESS_RESUME : AMA_ACCESS_ENTRY);
            if (scenario == 2) { require(result.outcome == AMA_HOST_FAILED && result.status.code == AMA_NATIVE_FAILURE); }
            else { require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK); }
            require(native_ready(1));
            current_sender = true;
            result = ama_project_execute(&producer, AMA_ACCESS_RESUME);
            require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
            if (scenario != 2) {
                dispose_actor(&producer, false);
                current_sender = false;
                result = ama_project_execute(&receiver, AMA_ACCESS_ENTRY);
                require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
            }
        }
        dispose_actor(&producer, scenario == 1);
        dispose_actor(&receiver, scenario == 2);
        require(host_sender.state == NULL && host_receiver.state == NULL);
        for (size_t index = 0; index < 2; index++) {
            if (observed[index].state != NULL) {
                require(ama_native_event_acceptance(&observed[index]) == AMA_EVENT_SOURCE_CLOSED);
                ama_native_event_submission_drop(&observed[index]);
            }
        }
        require(live_allocations == identity_history);
    }
    return EXIT_SUCCESS;
}
