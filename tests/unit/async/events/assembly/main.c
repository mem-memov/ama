/** @file
 * @brief Execute the exact single-translation-unit native provider assembly contract.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * retains only native resources, independently of any Ama execution runtime.
 */
#include "../common.c"
#ifdef TEST_EVENT_RELEASE
/* Compile the real native component with release assertions disabled, then
 * restore the independent consumer's assertions. The leaf keeps its harness on. */
#define NDEBUG
#include <assert.h>
#endif
#include "native/events/assembly.h"
#include "native/events/assembly.h"
#ifdef TEST_EVENT_RELEASE
#undef NDEBUG
#include <assert.h>
#endif
/** The assembly composes once, publishes real bytes and preserves resource lifetime after close. */
static void it_executes_the_public_native_provider_assembly_once(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission submission = prepare(owner);
    AmaNativeEventBuffer buffer = {0};
    const unsigned char data[] = {'a', 0, 'b'};
    ok(ama_native_event_buffer_copy(&submission, (AmaByteView){data, sizeof(data)}, &buffer));
    ok(ama_native_event_service_arm(owner, &submission, pending, NULL));
    AmaNativeEventPostResult posted = ama_native_event_post(&submission, 41, &buffer);
    assert(posted == AMA_EVENT_QUEUED && buffer.owner == NULL);
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && completion.code == 41 && completion.buffer.length == sizeof(data));
    assert(memcmp(completion.buffer.data, data, sizeof(data)) == 0);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&submission);
    assert(atomic_load(&retirements) == 0);
    ama_native_event_completion_drop(&completion);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Register the actual SDK assembly consumer explicitly. */
void run(void) { it_executes_the_public_native_provider_assembly_once(); }
/** Return after inclusion, execution and native ownership all agree. */
int main(void) { run(); return 0; }
