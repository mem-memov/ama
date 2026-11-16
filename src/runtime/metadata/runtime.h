/**
 * @file
 * @brief Declare host phase and root/current local identities without a C activation stack.
 *
 * [RT-05](../../../spec/implementation/runtime.md#rt-05)
 * The runtime/context owner gates this metadata; generated hosts borrow it only
 * under checked admission. Scheduling remains an explicit library/host operation.
 */
#ifndef AMA_RUNTIME_METADATA_RUNTIME_H
#define AMA_RUNTIME_METADATA_RUNTIME_H

#include "host.h"

/** Synchronous host admission phase; failed execution cannot resume and yielded execution returns idle. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05). */
typedef enum {
    AMA_RUNTIME_IDLE,
    AMA_RUNTIME_EXECUTING,
    AMA_RUNTIME_TERMINATED
} AmaRuntimePhase;

/* Per-host runtime control, no independent activation stack or payload mirrors. */
/** Host root/current control without activation stack or state mirrors; runtime/context supplies independent ownership and access admission. Component contract: [RT-05](../../../spec/implementation/runtime.md#rt-05). */
typedef struct {
    AmaRuntimePhase phase;
    AmaHandle root;
    AmaHandle current;
} AmaRuntimeMetadata;

#endif
