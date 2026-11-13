/**
 * @file
 * @brief Own one checked native error-provider planning transaction before code emission.
 *
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) requires
 * collision-free library codes. Complete native identity keys determine order;
 * failed construction publishes neither provider rows nor numeric assignments.
 */
#include "internal.h"

/** Initialize a fresh one-shot compiler owner without allocating a provider or namespace. */
void native_status_providers_init(NativeStatusProviders *owner, SourceAllocator allocator) {
    *owner = (NativeStatusProviders){.initialized = true};
    source_arena_init(&owner->arena, allocator);
}

/** Release only owned planning scratch; every borrowed name remains in its source registry. */
void native_status_providers_destroy(NativeStatusProviders *owner) {
    if (!owner->initialized) { return; }
    source_arena_destroy(&owner->arena);
    *owner = (NativeStatusProviders){0};
}

/** Validate/prepare a complete private table, then atomically publish its immutable view.
 * Namespace capacity is checked before dereferencing inventory; an empty inventory is valid.
 */
AmaStatus native_status_providers_build(NativeStatusProviders *owner,
    const IdentityName *const *names, size_t count) {
    if (!owner->initialized || owner->attempted) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    owner->attempted = true;
    if (count > UINT16_MAX || count > SIZE_MAX / sizeof(NativeStatusProvider)) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    if (count != 0 && names == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    NativeStatusProvider *providers = count == 0 ? NULL
        : source_arena_allocate(&owner->arena, count * sizeof(*providers));
    if (count != 0 && providers == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    for (size_t index = 0; index < count; index++) {
        const IdentityName *name = names[index];
        if (name == NULL || name->category != IDENTITY_NATIVE_NAMESPACE
            || name->encoding == NULL || name->encoding_length == 0) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        providers[index] = (NativeStatusProvider){.name = name};
    }
    native_status_providers_order(providers, count);
    for (size_t index = 0; index < count; index++) {
        if (index != 0 && native_status_provider_compare(providers[index - 1].name, providers[index].name) == 0) {
            owner->conflict_first = providers[index - 1].name;
            owner->conflict_second = providers[index].name;
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        AmaStatus status = native_status_provider_ordinal(index, &providers[index].namespace_id);
        if (status.code != AMA_OK) { return status; }
    }
    owner->providers = providers;
    owner->count = count;
    owner->ready = true;
    return ama_status_success();
}
