/** @file
 * @brief Bridge component code pins to independently retiring native callback resources.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) keeps the image
 * resident through the final release; [ASYNC-09](../../../spec/language/async-and-parallelism.md#async-09)
 * allows native resources to outlive a detached Ama owner.
 */
#include <assert.h>
#include "native.h"

/** Consume an existing native credit; the external loader barrier covers return from this function. */
static void native_code_release(void *resource) {
    bool released = ama_library_code_release(resource);
    assert(released);
    (void)released;
}
/** Retain without allocating or inspecting Ama state; exhaustion preserves the initialized-empty output. */
static AmaStatus native_code_retain(void *resource, AmaNativeCodeLease *output) {
    if (resource == NULL || output == NULL || output->resource != NULL || output->release != NULL) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    if (!ama_library_code_retain(resource)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    *output = (AmaNativeCodeLease){.resource = resource, .release = native_code_release};
    return ama_status_success();
}

/** An invalid null counter remains an invalid supplied provider, never an implicit process-resident fallback. */
AmaNativeCodeProvider ama_library_code_provider(AmaLibraryCodePins *code) {
    return (AmaNativeCodeProvider){.resource = code, .retain = native_code_retain};
}
