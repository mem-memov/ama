/**
 * @file
 * @brief Declare host phase and root/current local identities without a C activation stack.
 *
 * [RT-05](../../../spec/implementation/runtime.md#rt-05)
 * This representation is not a scheduler or a thread-safe context API; explicit context ownership remains T006.
 */
#ifndef AMA_RUNTIME_METADATA_RUNTIME_H
#define AMA_RUNTIME_METADATA_RUNTIME_H

#include "host.h"

/** Synchronous host admission phase; failed execution cannot resume and yielded execution returns idle. */
typedef enum {
    AMA_RUNTIME_IDLE,
    AMA_RUNTIME_EXECUTING,
    AMA_RUNTIME_TERMINATED
} AmaRuntimePhase;

/* Per-host runtime control, no independent activation stack or payload mirrors. */
/** Host root/current control without activation stack or state mirrors; independent checked context ownership remains future work. */
typedef struct {
    AmaRuntimePhase phase;
    AmaHandle root;
    AmaHandle current;
} AmaRuntimeMetadata;

#endif
