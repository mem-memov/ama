/** @file
 * @brief Supervise independent channel actors on two real executor threads with native-only coordination.
 * [PAR-06](../../spec/language/async-and-parallelism.md#par-06) makes
 * wider shutdown explicit. Setup transfers endpoint owners before concurrency;
 * shared coordination contains no Ama references or views.
 */
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include "native/channels/channel.h"
static _Atomic size_t live_allocations;
static size_t identity_history;
static AmaNativeChannelSender host_sender;
static AmaNativeChannelReceiver host_receiver;
static AmaNativeEventSubmission observed[2];
static bool current_sender;
static unsigned policy;
static pthread_mutex_t control = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;
static unsigned entered;
static bool concurrent, receiver_returned;
/** Underlying allocator symbols are deliberately counted across both executor threads. */
void *__real_malloc(size_t);
/** Underlying zeroing allocator keeps compiler-folded allocations in the same accounting domain. */
void *__real_calloc(size_t, size_t);
/** Underlying deallocator runs only after the wrapper consumes its counted ownership credit. */
void __real_free(void *);
/** Check actual release behavior even when generated code disables assertions. */
static void require(bool value) { if (!value) { abort(); } assert(value); }
/** Each successful allocation adds one independent native/runtime ownership credit. */
void *__wrap_malloc(size_t size) {
    void *memory = __real_malloc(size);
    if (memory != NULL) { atomic_fetch_add(&live_allocations, 1); }
    return memory;
}
/** Include zeroing allocations introduced by optimizing C compilers. */
void *__wrap_calloc(size_t count, size_t size) {
    void *memory = __real_calloc(count, size);
    if (memory != NULL) { atomic_fetch_add(&live_allocations, 1); }
    return memory;
}
/** Release consumes exactly one actual allocation, never a guessed aggregate. */
void __wrap_free(void *memory) {
    if (memory != NULL) { require(atomic_fetch_sub(&live_allocations, 1) != 0); }
    __real_free(memory);
}
/** Factory setup is serialized before executor threads exist; the payload receives the unique right. */
void test_channel_claim_sender(AmaNativeChannelSender *output) {
    if (current_sender) { *output = host_sender; host_sender = (AmaNativeChannelSender){0}; }
}
/** Distinct receiver ownership is transferred during its context's explicit setup. */
void test_channel_claim_receiver(AmaNativeChannelReceiver *output) {
    if (!current_sender) { *output = host_receiver; host_receiver = (AmaNativeChannelReceiver){0}; }
}
/** Native configuration is copied into each probe at construction, never selected while executing. */
bool test_supervision_factory_role(void) { return current_sender; }
/** Capture native rights only; producer registers during setup, receiver only on its own executor. */
void test_wait_observe(const AmaNativeEventSubmission *source) {
    unsigned index = current_sender ? 1 : 0;
    ama_native_event_submission_drop(&observed[index]);
    require(ama_native_event_submission_clone(source, &observed[index]).code == AMA_OK);
}
/** Synchronize both native entries to prove simultaneous independent context admission. */
static void meet(void) {
    require(pthread_mutex_lock(&control) == 0);
    entered++;
    require(pthread_cond_broadcast(&changed) == 0);
    while (entered != 2) { require(pthread_cond_wait(&changed, &control) == 0); }
    require(pthread_mutex_unlock(&control) == 0);
}
/** A selected receiver failure does not implicitly decide the peer's cancellation policy. */
bool test_supervision_failure(bool sending) {
    if (!concurrent || sending) { return false; }
    meet();
    return policy != 0;
}
/** The supervisor explicitly chooses continuation or cooperative peer cancellation after receiver return. */
bool test_channel_should_cancel(void) {
    if (!concurrent) { return false; }
    meet();
    require(pthread_mutex_lock(&control) == 0);
    while (!receiver_returned) { require(pthread_cond_wait(&changed, &control) == 0); }
    require(pthread_mutex_unlock(&control) == 0);
    return policy == 2;
}
/** Cancellation and ordinary send failure remain different task outcomes. */
unsigned test_channel_expected_outcome(void) { return policy == 2 ? 1u : 2u; }
/** Continue-on-failure observes receiver abort as ordinary data in the surviving sender. */
bool test_channel_expected_failure(void) { return policy == 1; }
/** Concurrent trace order is intentionally unspecified; typed observers still check every value. */
static int discard_trace(const char *text) { (void)text; return 0; }
#define puts discard_trace
#include "project.c"
#undef puts

/** A host worker owns exactly one context and its native readiness observation. */
typedef struct { AmaProjectHost host; bool sending; AmaHostResult result; } Actor;
/** Read readiness through native ownership, without inspecting either actor's Ama fields. */
static bool ready(unsigned index) {
    bool value = false;
    require(ama_native_event_ready(&observed[index], &value).code == AMA_OK);
    return value;
}
/** Publish a dispatch boundary before waiting; predicate and sleep share the supervisor mutex. */
static void notify_return(bool receiving) {
    require(pthread_mutex_lock(&control) == 0);
    if (receiving) { receiver_returned = true; }
    require(pthread_cond_broadcast(&changed) == 0);
    require(pthread_mutex_unlock(&control) == 0);
}
/** Execute/resume only the owned context. Native callbacks never enter this function. */
static void *execute_actor(void *resource) {
    Actor *actor = resource;
    actor->result = ama_project_execute(&actor->host, actor->sending ? AMA_ACCESS_RESUME : AMA_ACCESS_ENTRY);
    notify_return(!actor->sending);
    while (actor->result.outcome == AMA_HOST_YIELDED) {
        require(pthread_mutex_lock(&control) == 0);
        while (!ready(actor->sending ? 1 : 0)) { require(pthread_cond_wait(&changed, &control) == 0); }
        require(pthread_mutex_unlock(&control) == 0);
        actor->result = ama_project_execute(&actor->host, AMA_ACCESS_RESUME);
        notify_return(!actor->sending);
    }
    return NULL;
}
/** Setup fills capacity then drains once, leaving an explicit suspended producer and reusable receiver. */
static void exercise_policy(void) {
    entered = 0; receiver_returned = false; concurrent = false;
    const AmaNativeCodeProvider resident = {0};
    require(ama_native_channel_create(&test_message_schema, 1, 512, &resident, &host_sender, &host_receiver).code == AMA_OK);
    Actor producer = {.sending=true}, receiver = {0};
    current_sender = true;
    AmaHostResult result = ama_project_execute(&producer.host, AMA_ACCESS_ENTRY);
    require(result.outcome == AMA_HOST_YIELDED);
    current_sender = false;
    result = ama_project_execute(&receiver.host, AMA_ACCESS_ENTRY);
    require(result.outcome == AMA_HOST_COMPLETED && ready(1));
    require(host_sender.state == NULL && host_receiver.state == NULL);
    concurrent = true;
    pthread_t sender_thread, receiver_thread;
    require(pthread_create(&sender_thread, NULL, execute_actor, &producer) == 0);
    require(pthread_create(&receiver_thread, NULL, execute_actor, &receiver) == 0);
    require(pthread_join(sender_thread, NULL) == 0);
    require(pthread_join(receiver_thread, NULL) == 0);
    concurrent = false;
    require(entered == 2 && receiver_returned);
    require(producer.result.outcome == AMA_HOST_COMPLETED && producer.result.status.code == AMA_OK);
    require(receiver.result.outcome == (policy == 0 ? AMA_HOST_COMPLETED : AMA_HOST_FAILED));
    require(receiver.result.status.code == (policy == 0 ? AMA_OK : AMA_NATIVE_FAILURE));
    require(ama_project_dispose(&producer.host).code == AMA_OK);
    require(ama_project_dispose(&receiver.host).code == (policy == 0 ? AMA_OK : AMA_NATIVE_FAILURE));
    for (size_t index = 0; index < 2; index++) {
        if (observed[index].state != NULL) {
            require(ama_native_event_acceptance(&observed[index]) == AMA_EVENT_SOURCE_CLOSED);
            ama_native_event_submission_drop(&observed[index]);
        }
    }
    require(atomic_load(&live_allocations) == identity_history);
}
/** Verify normal delivery and two different deliberate policies for the same isolated failure. */
int main(void) {
    AmaNativeEventService *bootstrap = NULL;
    require(ama_native_event_service_create(1, NULL, &bootstrap).code == AMA_OK);
    ama_native_event_service_dispose(&bootstrap);
    identity_history = atomic_load(&live_allocations);
    require(identity_history == 1);
    for (policy = 0; policy < 3; policy++) { exercise_policy(); }
    puts("parallel supervision accepted");
    return EXIT_SUCCESS;
}
