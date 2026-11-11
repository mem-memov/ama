#ifndef AMA_RUNTIME_METADATA_IDENTITY_H
#define AMA_RUNTIME_METADATA_IDENTITY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "native/status/status.h"

typedef struct {
    uint64_t type_id;
    size_t slot_id;
    uint64_t generation;
} AmaHandle;

typedef struct {
    uint64_t type_id;
    size_t slot_id;
    uint64_t generation;
    bool live;
    bool retired;
} AmaSlotIdentity;

bool ama_handle_is_empty(AmaHandle handle);
AmaStatus ama_validate_handle(AmaHandle handle, uint64_t expected_type,
    const AmaSlotIdentity *slot);
AmaStatus ama_slot_advance_generation(AmaSlotIdentity *slot);

#endif
