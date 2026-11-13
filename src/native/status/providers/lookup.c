/**
 * @file
 * @brief Resolve a native provider namespace through exact complete-key binary search.
 *
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) prevents
 * duplicate provider codes. Lookup borrows an immutable sorted plan and changes
 * output only after full key equality, never digest or pointer coincidence.
 */
#include "internal.h"

/** Halve the remaining finite interval until an equal key is found or its absence is proved. */
AmaStatus native_status_provider_find(const NativeStatusProviders *owner,
    const IdentityName *name, uint16_t *namespace_id) {
    if (!owner->initialized || !owner->ready || namespace_id == NULL || name == NULL
        || name->category != IDENTITY_NATIVE_NAMESPACE || name->encoding == NULL || name->encoding_length == 0) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    size_t lower = 0, upper = owner->count;
    while (lower < upper) {
        size_t middle = lower + (upper - lower) / 2;
        int compared = native_status_provider_compare(name, owner->providers[middle].name);
        if (compared == 0) {
            *namespace_id = owner->providers[middle].namespace_id;
            return ama_status_success();
        }
        if (compared < 0) { upper = middle; }
        else { lower = middle + 1; }
    }
    return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0);
}
