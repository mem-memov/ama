/** @file
 * @brief Give separate generated contexts independent native sender/receiver ownership and count all allocations.
 * Endpoint injection is native host control only; no Ama handle, view or payload address is retained.
 */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "native/channels/channel.h"
static size_t live_allocations;
static AmaNativeChannelSender host_sender;
static AmaNativeChannelReceiver host_receiver;
static bool current_sender;
/** Original allocator behind the generated project's explicit wrap boundary. */
void *__real_malloc(size_t);
/** Original zeroing allocator, including legal compiler folding of malloc plus initialization. */
void *__real_calloc(size_t, size_t);
/** Original release, independent of native/Ama resource accounting. */
void __real_free(void *);
/** Count only direct project allocations; libc internal state is outside this boundary. */
void *__wrap_malloc(size_t size) {
    void *memory = __real_malloc(size);
    if (memory != NULL) { live_allocations++; }
    return memory;
}
/** Count actual optimized zeroing allocations instead of disabling compiler transformations. */
void *__wrap_calloc(size_t count, size_t size) {
    void *memory = __real_calloc(count, size);
    if (memory != NULL) { live_allocations++; }
    return memory;
}
/** Require exact cleanup, including queue storage surviving sender-context destruction. */
void __wrap_free(void *memory) {
    if (memory != NULL) {
        if (live_allocations == 0) { fputs("Channel consumer released an uncounted allocation\n", stderr); abort(); }
        live_allocations--;
    }
    __real_free(memory);
}
/** Factory moves one explicit host-owned right into the matching native payload. */
void test_channel_claim_sender(AmaNativeChannelSender *output) {
    if (current_sender) { *output = host_sender; host_sender = (AmaNativeChannelSender){0}; }
}
/** The other context receives the distinct receiver, never a copied sender or Ama reference. */
void test_channel_claim_receiver(AmaNativeChannelReceiver *output) {
    if (!current_sender) { *output = host_receiver; host_receiver = (AmaNativeChannelReceiver){0}; }
}
/** Select a test actor from native host setup; generated state selection stays explicit. */
bool test_channel_role(void) { return current_sender; }
#include "project.c"
/** Keep test checks effective in release-generated consumers. */
static void require_at(bool value, const char *expression, int line) {
    if (!value) { fprintf(stderr, "Channel consumer:%d: failed %s (live=%zu)\n", line, expression, live_allocations); abort(); }
    assert(value);
}
#define require(value) require_at((value), #value, __LINE__)
/** Full is ordinary data, receipt frees capacity, and accepted data survives sender disposal. */
int main(void) {
    const AmaNativeCodeProvider resident = {0};
    require(ama_native_channel_create(&test_message_schema, 1, 512, &resident,
        &host_sender, &host_receiver).code == AMA_OK);
    AmaProjectHost producer = {0}, receiver = {0};
    current_sender = true;
    AmaHostResult result = ama_project_execute(&producer, AMA_ACCESS_ENTRY);
    require(result.outcome == AMA_HOST_YIELDED && result.status.code == AMA_OK && host_sender.state == NULL);
    current_sender = false;
    result = ama_project_execute(&receiver, AMA_ACCESS_ENTRY);
    require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK && host_receiver.state == NULL);
    current_sender = true;
    result = ama_project_execute(&producer, AMA_ACCESS_RESUME);
    require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
    require(ama_project_dispose(&producer).code == AMA_OK);
    current_sender = false;
    result = ama_project_execute(&receiver, AMA_ACCESS_ENTRY);
    require(result.outcome == AMA_HOST_COMPLETED && result.status.code == AMA_OK);
    require(ama_project_dispose(&receiver).code == AMA_OK);
    require(live_allocations == 0);
    return EXIT_SUCCESS;
}
