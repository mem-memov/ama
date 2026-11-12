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

/** Lifecycle progression independent of publication and execution activity; Finalized may still have cleanup credits to consume. */
typedef enum {
    AMA_LIFETIME_FRESH,
    AMA_LIFETIME_CONSTRUCTING,
    AMA_LIFETIME_LIVE,
    AMA_LIFETIME_FINALIZED
} AmaLifetimePhase;

/** Exactly one selected constructor kind; a clone/literal never implicitly runs default construction first. */
typedef enum {
    AMA_CONSTRUCT_DEFAULT,
    AMA_CONSTRUCT_LITERAL,
    AMA_CONSTRUCT_CLONE
} AmaConstructorKind;

/* Runtime-owned ledger, never part of an opaque native payload.
 * live_children is a constructed declaration-order prefix; cleanup consumes it.
 */
/** Runtime-owned resource ledger: live_children is a declaration-order completed prefix, native_live is one destructor credit, cleanup_active prevents duplicate cleanup. */
typedef struct {
    AmaLifetimePhase phase;
    AmaConstructorKind constructor;
    size_t child_capacity;
    size_t live_children;
    bool native_live;
    bool cleanup_active;
} AmaLifetime;

/** Require fresh, unowned metadata and a valid selected constructor; record expected child capacity without invoking a hook. Invalid state leaves the ledger unchanged. */
AmaStatus ama_lifetime_begin(AmaLifetime *lifetime, size_t children, AmaConstructorKind kind);
/** Credit one successfully initialized child in declaration order. Require constructing state and unused prefix capacity; never credit a failed child. */
AmaStatus ama_lifetime_child_ready(AmaLifetime *lifetime);
/** Publish lifecycle Live only after the entire child prefix is complete. The caller supplies whether one native resource was successfully constructed. */
AmaStatus ama_lifetime_finish(AmaLifetime *lifetime, bool native_constructed);
/** Enter cleanup from Live or Constructing exactly once and mark Finalized before any user destructor can run. */
AmaStatus ama_lifetime_begin_cleanup(AmaLifetime *lifetime);
/** Consume the native-live bit before returning true, preventing duplicate destructor invocation even when that destructor fails. */
bool ama_lifetime_take_native(AmaLifetime *lifetime);
/** After native cleanup, consume the next reverse child index from the initialized prefix; false leaves output unchanged. */
bool ama_lifetime_next_child(AmaLifetime *lifetime, size_t *index);
/** Finish only after native and child credits are exhausted; no allocation or payload access. */
AmaStatus ama_lifetime_end_cleanup(AmaLifetime *lifetime);

#endif
