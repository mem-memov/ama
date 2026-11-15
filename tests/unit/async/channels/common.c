/** @file
 * @brief Own atomic native allocation/code counters and a fixed typed sequence message schema.
 * Fault injection runs without worker concurrency; successful stress accounting remains atomic.
 */
#ifndef AMA_CHANNEL_TEST_COMMON_C
#define AMA_CHANNEL_TEST_COMMON_C
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <string.h>
#include <threads.h>
#include "native/channels/channel.h"

static _Atomic size_t allocation_count, fail_at, live_allocations, code_credits;
static _Atomic bool refuse_code;
/** Original allocator behind the explicit native test wrap. */
void *__real_malloc(size_t);
/** Original zeroing allocation, including compiler-folded native initialization. */
void *__real_calloc(size_t, size_t);
/** Original release, called after atomically consuming the counted allocation. */
void __real_free(void *);
/** Inject one deterministic allocation failure without changing prior owners. */
void *__wrap_malloc(size_t size) {
    size_t number = atomic_fetch_add(&allocation_count, 1) + 1;
    if (number == atomic_load(&fail_at)) { return NULL; }
    void *memory = __real_malloc(size);
    if (memory != NULL) { atomic_fetch_add(&live_allocations, 1); }
    return memory;
}
/** Fault/count the actual optimized allocator without imposing a source spelling on the compiler. */
void *__wrap_calloc(size_t count, size_t size) {
    size_t number = atomic_fetch_add(&allocation_count, 1) + 1;
    if (number == atomic_load(&fail_at)) { return NULL; }
    void *memory = __real_calloc(count, size);
    if (memory != NULL) { atomic_fetch_add(&live_allocations, 1); }
    return memory;
}
/** Native queue releases may run on either endpoint's thread. */
void __wrap_free(void *memory) {
    if (memory != NULL) {
        size_t previous = atomic_fetch_sub(&live_allocations, 1);
        assert(previous > 0);
    }
    __real_free(memory);
}
/** Creator-image bookkeeping is independent of both endpoint/context lifetimes. */
static void code_release(void *resource) {
    assert(resource == &code_credits);
    size_t previous = atomic_fetch_sub(&code_credits, 1);
    assert(previous > 0);
}
/** Every accepted native owner receives exactly one move-only credit. */
static AmaStatus code_retain(void *resource, AmaNativeCodeLease *output) {
    assert(resource == &code_credits);
    if (atomic_load(&refuse_code)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    atomic_fetch_add(&code_credits, 1);
    *output = (AmaNativeCodeLease){resource, code_release};
    return ama_status_success();
}
static const AmaNativeCodeProvider provider = {&code_credits, code_retain};
static const unsigned char protocol_name[] = "channel";
static const AmaNativeMessageField field = {AMA_MESSAGE_BYTES, false, 1, 8};
static const AmaNativeMessageSchema schema = {{protocol_name, sizeof(protocol_name) - 1}, 1, &field, 1, 256};

/** Fail independently on technical status or an unexpected ordinary outcome. */
static void channel_reason(AmaNativeChannelResult result, AmaNativeChannelReason reason) {
    assert(result.status.code == AMA_OK && result.reason == reason);
}
/** Check native lifecycle/configuration status without treating it as a data result. */
static void native_ok(AmaStatus status) { assert(status.code == AMA_OK); }
/** Encode one mathematical sequence into independently owned data bytes. */
static AmaNativeMessageBytes sequence_message(uint64_t sequence) {
    unsigned char bytes[8];
    for (size_t index = 8; index != 0;) { bytes[--index] = (unsigned char)sequence; sequence >>= 8; }
    AmaNativeMessageValue value = {0, {bytes, sizeof(bytes)}};
    AmaNativeMessageFieldValue values = {&value, 1};
    AmaNativeMessageBytes output = {0};
    AmaNativeMessageResult result = ama_native_message_encode(&schema, &values, &provider, &output);
    assert(result.status.code == AMA_OK && result.reason == AMA_MESSAGE_ACCEPTED);
    return output;
}
/** Decode after receipt independently of whether either channel endpoint still exists. */
static uint64_t message_sequence(const AmaNativeMessageBytes *message) {
    AmaNativeMessageDecoded output = {0};
    AmaNativeMessageResult result = ama_native_message_decode(&schema,
        (AmaByteView){message->data, message->length}, &provider, &output);
    assert(result.status.code == AMA_OK && result.reason == AMA_MESSAGE_ACCEPTED);
    AmaByteView bytes = output.fields[0].items[0].bytes;
    assert(bytes.length == 8);
    uint64_t value = 0;
    for (size_t index = 0; index < bytes.length; index++) { value = (value << 8) | bytes.data[index]; }
    ama_native_message_decoded_drop(&output);
    return value;
}
/** No native allocation or code credit may survive a scenario. */
static void no_resources(void) {
    assert(atomic_load(&live_allocations) == 0 && atomic_load(&code_credits) == 0);
}
#endif
