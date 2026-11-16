/**
 * @file
 * @brief Assign component-local native error namespaces from complete collision-checked identities.
 *
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) prevents
 * participating native libraries from allocating colliding codes. Planning owns
 * only a sorted pointer table; complete immutable native identities remain in
 * their source registry. [NAME-05](../../../../spec/implementation/native-interface.md#name-05)
 * includes root kind in those keys. This compiler owner is not mutable Ama state.
 */
#ifndef AMA_NATIVE_STATUS_PROVIDERS_H
#define AMA_NATIVE_STATUS_PROVIDERS_H
#include "source/identity/identity.h"
#include "native/status/status.h"

/** One borrowed complete native provider identity and its assigned nonzero 16-bit namespace. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
typedef struct {
    const IdentityName *name;
    uint16_t namespace_id;
} NativeStatusProvider;

/** Compiler-owned one-shot planning owner. Arena scratch is private until ready;
 * names and their encodings come from one live collision-checked registry and
 * outlive this owner. Conflict pointers borrow the same inventory on duplicate failure.
 * Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
typedef struct {
    SourceArena arena;
    const NativeStatusProvider *providers;
    size_t count;
    const IdentityName *conflict_first;
    const IdentityName *conflict_second;
    bool initialized;
    bool attempted;
    bool ready;
} NativeStatusProviders;

/** Initialize fresh compiler storage with borrowed allocation authority; create no provider ID yet. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
void native_status_providers_init(NativeStatusProviders *, SourceAllocator);
/** Release complete/failed private scratch and clear ownership; borrowed identities are not freed. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
void native_status_providers_destroy(NativeStatusProviders *);
/** Plan once: validate native keys, sort complete encodings and assign ordinal plus one.
 * Reject duplicates, malformed inventory, allocation failure or more than 65535
 * providers without publishing any table. Failure scratch remains owned until destroy.
 * Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
AmaStatus native_status_providers_build(NativeStatusProviders *, const IdentityName *const *, size_t);
/** Look up an exact complete native key in a ready plan. Output changes only on success;
 * hash/name-pointer equality is never substituted for full encoding equality.
 * Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
AmaStatus native_status_provider_find(const NativeStatusProviders *, const IdentityName *, uint16_t *);
#endif
