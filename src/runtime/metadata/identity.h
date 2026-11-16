/**
 * @file
 * @brief Declare exact context-local nominal slot and generation identities.
 *
 * [REF-02](../../../spec/language/values-and-references.md#ref-02)
 * Handles borrow no C address. The runtime/context gateway checks host origin;
 * public compiled-component transports authenticate their separate identities
 * before resolving these context-local triples.
 */
#ifndef AMA_RUNTIME_METADATA_IDENTITY_H
#define AMA_RUNTIME_METADATA_IDENTITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "native/status/status.h"

/** Exact context-local nominal identity; generation zero is empty and slot zero is valid. No context authentication or payload pointer is included. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
typedef struct {
    uint64_t type_id;
    size_t slot_id;
    uint64_t generation;
} AmaHandle;

/** Slot publication and permanent retirement history; releasing payload cannot reset a previously issued identity. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
typedef struct {
    uint64_t type_id;
    size_t slot_id;
    uint64_t generation;
    bool live;
    bool retired;
} AmaSlotIdentity;

/** Compare all three local identity fields exactly. Equality does not validate liveness or authenticate an execution context. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
static inline bool ama_handle_equal(AmaHandle left, AmaHandle right) {
    return left.type_id == right.type_id && left.slot_id == right.slot_id &&
        left.generation == right.generation;
}

/** Treat generation zero as empty independently of other bits; the canonical produced empty value is all zero. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
bool ama_handle_is_empty(AmaHandle handle);
/** Check emptiness, nominal type, resolved slot, publication and exact generation before any payload access. The caller supplies a valid bounded slot pointer or null. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
AmaStatus ama_validate_handle(AmaHandle handle, uint64_t expected_type,
    const AmaSlotIdentity *slot);
/** Require a nonlive, nonretired slot and valid nonzero limit. Advance strictly or retire permanently at the limit; never wrap or revive an identity. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
AmaStatus ama_slot_advance_generation_limited(AmaSlotIdentity *slot, uint64_t limit);
/** Apply generation advancement with UINT64_MAX as the retirement boundary. Component contract: [REF-02](../../../spec/language/values-and-references.md#ref-02). */
AmaStatus ama_slot_advance_generation(AmaSlotIdentity *slot);

#endif
