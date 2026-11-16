/**
 * @file
 * @brief Declare the lifecycle ledger independently of payload and execution activity.
 *
 * [LIFE-01](../../../spec/implementation/native-interface.md#life-01)
 * All operations borrow writable metadata; the ledger owns no memory and cannot invoke native hooks itself.
 */
#ifndef AMA_RUNTIME_LIFETIME_H
#define AMA_RUNTIME_LIFETIME_H

#include <stdbool.h>
#include <stddef.h>
#include "native/status/status.h"

/** Lifecycle progression independent of publication and execution activity; Finalized may still have cleanup credits to consume. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
typedef enum {
    AMA_LIFETIME_FRESH,
    AMA_LIFETIME_CONSTRUCTING,
    AMA_LIFETIME_LIVE,
    AMA_LIFETIME_FINALIZED
} AmaLifetimePhase;

/** Exactly one selected constructor kind, including the fixed integer size bridge;
 * clone, literal and size construction never run the default payload constructor first.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
typedef enum {
    AMA_CONSTRUCT_DEFAULT,
    AMA_CONSTRUCT_LITERAL,
    AMA_CONSTRUCT_CLONE,
    AMA_CONSTRUCT_SIZE,
    /** Compiler-owned typed C transport decoder; never a new native .abi binding. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
    AMA_CONSTRUCT_TRANSPORT
} AmaConstructorKind;

/* Runtime-owned ledger, never part of an opaque native payload.
 * live_children is a constructed declaration-order prefix; cleanup consumes it.
 */
/** Runtime-owned resource ledger, accessible only through the runtime owner.
 * Every reachable state satisfies live_children <= child_capacity. Constructing
 * owns no native credit; finish records one only after all children are ready.
 * Cleanup implies Finalized and consumes credits before callbacks, so destructor
 * failure cannot restore ownership or authorize another attempt. Reuse requires
 * a newly reserved identity and fresh ledger, never restarting a finalized value.
 * Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
typedef struct {
    AmaLifetimePhase phase;
    AmaConstructorKind constructor;
    size_t child_capacity;
    size_t live_children;
    bool native_live;
    bool cleanup_active;
} AmaLifetime;

/** Require fresh, unowned metadata and a valid selected constructor; record expected child capacity without invoking a hook. Invalid state leaves the ledger unchanged. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
AmaStatus ama_lifetime_begin(AmaLifetime *lifetime, size_t children, AmaConstructorKind kind);
/** Credit one successfully initialized child in declaration order. Require constructing state and unused prefix capacity; never credit a failed child. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
AmaStatus ama_lifetime_child_ready(AmaLifetime *lifetime);
/** Publish lifecycle Live only after the entire child prefix is complete. The caller supplies whether one native resource was successfully constructed. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
AmaStatus ama_lifetime_finish(AmaLifetime *lifetime, bool native_constructed);
/** Enter cleanup from Live or Constructing exactly once and mark Finalized before any user destructor can run. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
AmaStatus ama_lifetime_begin_cleanup(AmaLifetime *lifetime);
/** Consume the native-live bit before returning true, preventing duplicate destructor invocation even when that destructor fails. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool ama_lifetime_take_native(AmaLifetime *lifetime);
/** After native cleanup, consume the next reverse child index from the initialized prefix; false leaves output unchanged. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
bool ama_lifetime_next_child(AmaLifetime *lifetime, size_t *index);
/** Finish only after native and child credits are exhausted; no allocation or payload access. Component contract: [LIFE-01](../../../spec/implementation/native-interface.md#life-01). */
AmaStatus ama_lifetime_end_cleanup(AmaLifetime *lifetime);

#endif
