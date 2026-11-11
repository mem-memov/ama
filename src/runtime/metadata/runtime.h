#ifndef AMA_RUNTIME_METADATA_RUNTIME_H
#define AMA_RUNTIME_METADATA_RUNTIME_H

#include "host.h"

typedef enum {
    AMA_RUNTIME_IDLE,
    AMA_RUNTIME_EXECUTING,
    AMA_RUNTIME_TERMINATED
} AmaRuntimePhase;

/* Per-host runtime control, no independent activation stack or payload mirrors. */
typedef struct {
    AmaRuntimePhase phase;
    AmaHandle root;
    AmaHandle current;
} AmaRuntimeMetadata;

#endif
