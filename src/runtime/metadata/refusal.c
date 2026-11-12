/**
 * @file
 * @brief Record explicit acquisition refusals and expose immutable native snapshots.
 *
 * [CALL-04](../../../spec/language/calls-and-results.md#call-04)
 * Technical errors cannot overwrite the last refusal. The native snapshot contains data, not runtime authority.
 */
#include "service.h"

/** Accept only explicit acquisition-refusal codes and replace this caller snapshot; technical errors and success leave the snapshot unchanged. */
AmaStatus ama_service_record_refusal(AmaServiceState *caller, AmaStatus refusal) {
    switch (refusal.code) {
        case AMA_INVALID_INDEX:
        case AMA_INDEX_BOUNDS:
        case AMA_TYPE_MISMATCH:
        case AMA_EMPTY_REFERENCE:
        case AMA_STALE_REFERENCE:
        case AMA_BUSY:
        case AMA_NO_RESUME:
            caller->last_refusal = refusal;
            return ama_status_success();
        default:
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
}

/** Copy the immediate caller refusal as immutable data, or success for a null host caller; expose no pointer to runtime controls. */
AmaNativeServices ama_native_services_snapshot(const AmaServiceState *caller) {
    AmaNativeServices services = {0};
    if (caller != NULL) {
        services.caller_refusal = caller->last_refusal;
    }
    return services;
}
