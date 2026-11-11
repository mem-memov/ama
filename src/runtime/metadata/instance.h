#ifndef AMA_RUNTIME_METADATA_INSTANCE_H
#define AMA_RUNTIME_METADATA_INSTANCE_H

#include "service.h"
#include "runtime/lifecycle/lifetime.h"

typedef struct {
    AmaSlotIdentity identity;
    AmaLifetime lifetime;
    AmaServiceState service;
} AmaInstanceMetadata;

static inline AmaStatus ama_instance_access(const AmaInstanceMetadata *instance, AmaAccessMode mode) {
    if (!instance->identity.live || instance->identity.retired ||
        instance->identity.type_id == 0 || instance->identity.generation == 0 ||
        instance->lifetime.phase != AMA_LIFETIME_LIVE) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    return ama_service_access(&instance->service, mode);
}

#endif
