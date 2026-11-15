/** @file
 * @brief Check binary payload ownership independently of any UTF-8 interpretation.
 * [ASYNC-05](../../../../spec/language/async-and-parallelism.md#async-05)
 * requires independent native byte ownership before borrowed completion bytes expire.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#define AMA_NATIVE(name) event_bytes_ ## name
#include "../../../../stdlib/modules/bytes.h"
/** Match the public SDK state alias while testing the native storage component directly. */
typedef struct event_bytes_State event_bytes_State;
/** Inject allocation failure without changing the destination's old immutable backing. */
static bool fail_allocate;
/** Track exact backing retirement, including allocation-free empty values. */
static size_t allocations;
/** Allocate one independently owned backing unless this test requests failure. */
static void *allocate(size_t length) {
    if (fail_allocate) { return NULL; }
    void *result = malloc(length);
    if (result != NULL) { allocations++; }
    return result;
}
/** Consume one successful backing allocation. */
static void release(void *pointer) { assert(allocations != 0); allocations--; free(pointer); }
#define AMA_BYTES_ALLOCATE(size) allocate(size)
#define AMA_BYTES_RELEASE(pointer) release(pointer)
#define AMA_BYTES_SHARE_LIMIT 3
#include "../../../../stdlib/modules/bytes+/native/storage.c"
#include "../../../../stdlib/modules/bytes+/native/values.c"
/** Counted bytes accept embedded zero and invalid Unicode, preserve aliases and release each share once. */
static void it_preserves_binary_values_across_clone_and_failed_replacement(void) {
    const unsigned char input[] = {0, 255, 192, 128};
    event_bytes_State value, copy, third, failed;
    AmaStatus status = event_bytes_initialize(&value);
    assert(status.code == AMA_OK && allocations == 0);
    status = event_bytes_replace(&value, (AmaByteView){input, sizeof(input)});
    assert(status.code == AMA_OK && allocations == 1);
    status = event_bytes_clone(&copy, &value);
    assert(status.code == AMA_OK && copy.backing == value.backing);
    status = event_bytes_clone(&third, &value);
    assert(status.code == AMA_OK);
    status = event_bytes_clone(&failed, &value);
    assert(status.code == AMA_LIMIT_EXCEEDED && !failed.live && failed.backing == NULL);
    AmaByteView bytes = {0};
    status = event_bytes_view(&value, &bytes);
    assert(status.code == AMA_OK && bytes.length == sizeof(input) && memcmp(bytes.data, input, sizeof(input)) == 0);
    fail_allocate = true;
    status = event_bytes_replace(&value, bytes);
    assert(status.code == AMA_OUT_OF_MEMORY && value.backing == copy.backing);
    fail_allocate = false;
    status = event_bytes_replace(&value, bytes);
    assert(status.code == AMA_OK && value.backing != copy.backing && allocations == 2);
    status = event_bytes_replace(&value, (AmaByteView){NULL, 1});
    assert(status.code == AMA_INVALID_LITERAL && allocations == 2);
    status = event_bytes_replace(&value, (AmaByteView){input, SIZE_MAX});
    assert(status.code == AMA_LIMIT_EXCEEDED && allocations == 2);
    bool condition = false;
    status = event_bytes_to_condition(&value, &condition);
    assert(status.code == AMA_OK && condition);
    status = event_bytes_replace(&value, (AmaByteView){0});
    assert(status.code == AMA_OK && allocations == 1);
    status = event_bytes_to_condition(&value, &condition);
    assert(status.code == AMA_OK && !condition);
    status = event_bytes_destroy(&copy); assert(status.code == AMA_OK && allocations == 1);
    status = event_bytes_destroy(&third); assert(status.code == AMA_OK && allocations == 0);
    status = event_bytes_destroy(&value); assert(status.code == AMA_OK);
    status = event_bytes_destroy(&value); assert(status.code == AMA_INVALID_STATE);
}
/** Execute the component's actual ownership operations. */
void run(void) { it_preserves_binary_values_across_clone_and_failed_replacement(); }
/** Exit only after every allocated backing is retired. */
int main(void) { run(); return 0; }
