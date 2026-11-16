/** @file
 * @brief Retain native-only code lifetime independently of Ama contexts and operation-scoped views.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) requires detached
 * callbacks to keep their creator image resident; [CABI-05](../../../spec/implementation/native-interface.md#cabi-05)
 * forbids retaining Ama state or execution authority in the native lease.
 */
#ifndef AMA_NATIVE_ABI_CODE_H
#define AMA_NATIVE_ABI_CODE_H
#include "native/status/status.h"

/** Move-only native code credit. The resource contains native code bookkeeping only;
 * release is infallible, cannot enter Ama, and runs under the loader's call barrier.
 * Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [CABI-05](../../../spec/implementation/native-interface.md#cabi-05). */
typedef struct {
    void *resource;
    void (*release)(void *);
} AmaNativeCodeLease;
/** Borrowed immutable native code provider. Both fields zero means process-resident
 * code; a supplied provider retains independently owned credits without Ama access.
 * Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [CABI-05](../../../spec/implementation/native-interface.md#cabi-05). */
typedef struct {
    void *resource;
    AmaStatus (*retain)(void *, AmaNativeCodeLease *);
} AmaNativeCodeProvider;

/** Acquire only into an empty owner. Provider failure preserves it; a missing
 * provider is the explicit process-resident mode and needs no release operation.
 * Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [CABI-05](../../../spec/implementation/native-interface.md#cabi-05). */
static inline AmaStatus ama_native_code_acquire(const AmaNativeCodeProvider *provider, AmaNativeCodeLease *output) {
    if (provider == NULL || output == NULL || output->resource != NULL || output->release != NULL
        || ((provider->resource == NULL) != (provider->retain == NULL))) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return provider->retain == NULL ? ama_status_success()
        : ama_status_checked(provider->retain(provider->resource, output));
}
/** Consume the credit before its creator release; repeated release of an empty owner is harmless. Component contract: [EMBED-06](../../../spec/implementation/c-library.md#embed-06), [CABI-05](../../../spec/implementation/native-interface.md#cabi-05). */
static inline void ama_native_code_drop(AmaNativeCodeLease *owner) {
    if (owner == NULL) { return; }
    AmaNativeCodeLease lease = *owner;
    *owner = (AmaNativeCodeLease){0};
    if (lease.release != NULL) { lease.release(lease.resource); }
}
#endif
