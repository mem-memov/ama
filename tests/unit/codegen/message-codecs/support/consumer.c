/** @file
 * @brief Count every generated runtime/native allocation through completed or failed typed decoding.
 * Native libc internals are outside the wrap; every direct project allocation is balanced at disposal.
 */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
static size_t live_allocations;
/** Original libc allocation reached through the linker's explicit wrap boundary. */
void *__real_malloc(size_t);
/** Original libc release; no Ama or native ownership bookkeeping is hidden here. */
void __real_free(void *);
/** Count successful direct allocations owned by actual generated runtime or native values. */
void *__wrap_malloc(size_t size) {
    void *memory = __real_malloc(size);
    if (memory != NULL) { live_allocations++; }
    return memory;
}
/** Release only a real native allocation; checks remain active under NDEBUG. */
void __wrap_free(void *memory) {
    if (memory != NULL) {
        if (live_allocations == 0) { abort(); }
        live_allocations--;
    }
    __real_free(memory);
}
#include "project.c"
/** Execute one declared scenario and require exact cleanup after every outcome. */
int main(void) {
    AmaProjectHost host = {0};
    AmaHostResult result = ama_project_execute(&host, AMA_ACCESS_ENTRY);
#ifdef TEST_TECHNICAL_FAILURE
    if (result.outcome != AMA_HOST_FAILED || result.status.code != AMA_NATIVE_FAILURE) { abort(); }
    AmaStatus status = ama_project_dispose(&host);
    if (status.code != AMA_NATIVE_FAILURE) { abort(); }
#else
    if (result.outcome != AMA_HOST_COMPLETED || result.status.code != AMA_OK) { abort(); }
    AmaStatus status = ama_project_dispose(&host);
    if (status.code != AMA_OK) { abort(); }
#endif
    if (live_allocations != 0) { abort(); }
    return EXIT_SUCCESS;
}
