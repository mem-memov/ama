/** @file
 * @brief Supply exact typed records and counted native/code ownership without an Ama execution runtime.
 * Every allocation belongs to a message owner or its private construction prefix.
 */
#ifndef AMA_MESSAGE_TEST_COMMON_C
#define AMA_MESSAGE_TEST_COMMON_C
#include <assert.h>
#include <stdio.h>
#include "native/messages/internal.h"
static size_t allocation_count, fail_at, live_allocations;
/** Linker-provided original allocator; wrappers delegate every successful request here. */
void *__real_malloc(size_t);
/** Linker-provided original release, independent of native ownership accounting. */
void __real_free(void *);

/** Fail a selected actual native allocation; preserve every previously published owner. */
void *__wrap_malloc(size_t size) {
    allocation_count++;
    if (allocation_count == fail_at) { return NULL; }
    void *memory = __real_malloc(size);
    if (memory != NULL) { live_allocations++; }
    return memory;
}
/** Count only nonempty credits; the test never frees borrowed input/schema storage. */
void __wrap_free(void *memory) {
    if (memory != NULL) { assert(live_allocations != 0); live_allocations--; }
    __real_free(memory);
}

/** Code bookkeeping is native-only and can outlive the source/schema or context. */
typedef struct { size_t credits, released; bool refuse; } CodeOwner;
static CodeOwner code;
/** Consume exactly one independent creator-image credit. */
static void release_code(void *resource) {
    CodeOwner *owner = resource;
    assert(owner->credits > 0);
    owner->credits--;
    owner->released++;
}
/** A rejected code credit preserves initialized-empty destination storage. */
static AmaStatus retain_code(void *resource, AmaNativeCodeLease *output) {
    CodeOwner *owner = resource;
    if (owner->refuse) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    owner->credits++;
    *output = (AmaNativeCodeLease){owner, release_code};
    return ama_status_success();
}
static const AmaNativeCodeProvider provider = {&code, retain_code};
static const unsigned char name[] = {'t', 'e', 's', 't'};
static const AmaNativeMessageField field_schema[] = {
    {AMA_MESSAGE_CONDITION, false, 1, 0}, {AMA_MESSAGE_INTEGER, false, 1, 16},
    {AMA_MESSAGE_TEXT, false, 1, 16}, {AMA_MESSAGE_BYTES, true, 3, 16}
};
static const AmaNativeMessageSchema schema = {{name, sizeof(name)}, 7, field_schema, 4, 1024};
static const unsigned char magnitude[] = {1, 0, 255}, text[] = {'a', 0, 0xd0, 0x90}, binary[] = {0, 255, 66};
static const AmaNativeMessageValue condition_value[] = {{1, {NULL, 0}}};
static const AmaNativeMessageValue integer_value[] = {{1, {magnitude, sizeof(magnitude)}}};
static const AmaNativeMessageValue text_value[] = {{0, {text, sizeof(text)}}};
static const AmaNativeMessageValue bytes_value[] = {{0, {binary, sizeof(binary)}}, {0, {NULL, 0}}};
static const AmaNativeMessageFieldValue fields[] = {
    {condition_value, 1}, {integer_value, 1}, {text_value, 1}, {bytes_value, 2}
};

/** Require ordinary acceptance without classifying application reasons by technical code. */
static void accepted(AmaNativeMessageResult result) {
    assert(result.status.code == AMA_OK && result.reason == AMA_MESSAGE_ACCEPTED);
}
/** All expected rejections preserve the independently initialized-empty decoded owner. */
static void rejected(AmaNativeMessageResult result, AmaNativeMessageReason reason, const AmaNativeMessageDecoded *output) {
    assert(result.status.code == AMA_OK && result.reason == reason);
    assert(output->fields == NULL && output->values == NULL && output->wire == NULL && output->field_count == 0);
    assert(output->code.resource == NULL && output->code.release == NULL);
}
#endif
