/** @file
 * @brief Share a C-only native event observer between independently compiled host source files.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) allows this
 * explicit compatible C dependency; its callback records events without entering Ama.
 */
#undef NDEBUG
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "probe.h"
#include "observer.h"

/** External host instrumentation, not mutable data shared by Ama contexts. */
static size_t depth, resources;

/** Count acquisition/retirement and ordinary helper entry/return using only scalar events. */
static void observe(uint32_t event) {
    switch (event) {
        case AMA_NESTED_HELPER_ENTER:
            assert(depth == 0);
            depth++;
            break;
        case AMA_NESTED_HELPER_LEAVE:
            assert(depth == 1);
            depth--;
            break;
        case AMA_NESTED_RESOURCE_CREATE:
            resources++;
            break;
        case AMA_NESTED_RESOURCE_DESTROY:
            assert(resources != 0);
            resources--;
            break;
        default:
            assert(false);
    }
}
void example_observe_start(void) {
    assert(depth == 0 && resources == 0);
    ama_nested_probe_v1_set(observe);
}
void example_returned(void) { assert(depth == 0); }
void example_observe_finish(void) {
    assert(depth == 0 && resources == 0);
    ama_nested_probe_v1_set(NULL);
}
