/**
 * @file
 * @brief Combine published identity, lifecycle and service eligibility checks.
 *
 * [RT-04](../../../spec/implementation/runtime.md#rt-04)
 * No payload is read and no acquisition is performed by the eligibility predicate.
 */
#ifndef AMA_RUNTIME_METADATA_INSTANCE_H
#define AMA_RUNTIME_METADATA_INSTANCE_H

#include "service.h"
#include "runtime/lifecycle/lifetime.h"

/** Metadata borrowed with a stable slot: identity gates access, lifecycle owns resource credits and service owns execution controls. */
typedef struct {
    AmaSlotIdentity identity;
    AmaLifetime lifetime;
    AmaServiceState service;
} AmaInstanceMetadata;

/** Require published nonretired identity and Live lifecycle before service eligibility; this check does not acquire storage. */
static inline AmaStatus ama_instance_access(const AmaInstanceMetadata *instance, AmaAccessMode mode) {
    if (!instance->identity.live || instance->identity.retired ||
        instance->identity.type_id == 0 || instance->identity.generation == 0 ||
        instance->lifetime.phase != AMA_LIFETIME_LIVE) {
        return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
    }
    return ama_service_access(&instance->service, mode);
}

#endif
