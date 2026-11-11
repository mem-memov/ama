#ifndef AMA_RUNTIME_LIFETIME_H
#define AMA_RUNTIME_LIFETIME_H

#include <stdbool.h>
#include <stddef.h>
#include "native/status/status.h"

typedef enum {
    AMA_LIFETIME_FRESH,
    AMA_LIFETIME_CONSTRUCTING,
    AMA_LIFETIME_LIVE,
    AMA_LIFETIME_FINALIZED
} AmaLifetimePhase;

typedef enum {
    AMA_CONSTRUCT_DEFAULT,
    AMA_CONSTRUCT_LITERAL,
    AMA_CONSTRUCT_CLONE
} AmaConstructorKind;

/* Runtime-owned ledger, never part of an opaque native payload.
 * live_children is a constructed declaration-order prefix; cleanup consumes it.
 */
typedef struct {
    AmaLifetimePhase phase;
    AmaConstructorKind constructor;
    size_t child_capacity;
    size_t live_children;
    bool native_live;
    bool cleanup_active;
} AmaLifetime;

AmaStatus ama_lifetime_begin(AmaLifetime *lifetime, size_t children, AmaConstructorKind kind);
AmaStatus ama_lifetime_child_ready(AmaLifetime *lifetime);
AmaStatus ama_lifetime_finish(AmaLifetime *lifetime, bool native_constructed);
AmaStatus ama_lifetime_begin_cleanup(AmaLifetime *lifetime);
bool ama_lifetime_take_native(AmaLifetime *lifetime);
bool ama_lifetime_next_child(AmaLifetime *lifetime, size_t *index);
AmaStatus ama_lifetime_end_cleanup(AmaLifetime *lifetime);

#endif
