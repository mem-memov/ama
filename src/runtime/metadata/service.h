/**
 * @file
 * @brief Separate transient return connections from saved resume positions and refusal history.
 *
 * [RT-09](../../../spec/implementation/runtime.md#rt-09)
 * The synchronous runtime owns these fields; declared values and native payloads cannot store hidden execution links.
 */
#ifndef AMA_RUNTIME_METADATA_SERVICE_H
#define AMA_RUNTIME_METADATA_SERVICE_H

#include "identity.h"
#include "native/abi/services.h"

/** Per-instance activity, separate from lifecycle phase and saved position; Acquired protects retained results without executing them. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
typedef enum {
    AMA_INSTANCE_FREE,
    AMA_INSTANCE_READY,
    AMA_INSTANCE_RUNNING,
    AMA_INSTANCE_WAITING,
    AMA_INSTANCE_YIELDED,
    AMA_INSTANCE_ACQUIRED,
    AMA_INSTANCE_FAILED
} AmaInstanceActivity;

/** Explicit acquisition intent; resume alone requires a saved position and never prepares new interface inputs. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
typedef enum { AMA_ACCESS_ENTRY, AMA_ACCESS_RESUME, AMA_ACCESS_READ } AmaAccessMode;

/** Transient immediate-caller connection or host sentinel. It is cleared on return and is never continuation state. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
typedef struct {
    bool attached;
    bool host;
    AmaHandle caller;
    uint64_t continuation;
    uint64_t call_site;
} AmaReturnLink;

/** Explicit saved selector; valid requires nonzero selector and generated dispatch separately checks label membership. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
typedef struct {
    bool valid;
    uint64_t selector;
} AmaResumePosition;

/* Runtime-owned controls, separate from declared values and native payload. */
/** Runtime-owned execution/refusal controls. outgoing retains one acquired callee until caller-side result processing releases it. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
typedef struct {
    AmaInstanceActivity activity;
    AmaReturnLink return_link;
    AmaResumePosition resume;
    uint64_t current_operation;
    AmaStatus last_refusal;
    AmaHandle outgoing;
    AmaInstanceActivity outgoing_previous;
} AmaServiceState;

/** Check the distinct zero unattached, host-root and ordinary caller encodings; selector membership and caller liveness are separate runtime checks. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
static inline bool ama_return_link_is_valid(AmaReturnLink link) {
    if (!link.attached || link.host) {
        return (!link.host || link.attached) && ama_handle_is_empty(link.caller) &&
            link.continuation == 0 && link.call_site == 0;
    }
    return link.caller.type_id != 0 && link.caller.generation != 0 &&
        link.continuation != 0 && link.call_site != 0;
}

/** Check a borrowed service record for entry/resume/read eligibility without mutation. Busy states refuse, and inconsistent ready/yielded metadata is technical failure. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
AmaStatus ama_service_access(const AmaServiceState *service, AmaAccessMode mode);
/** Accept only explicit acquisition-refusal codes with a valid message view and replace
 * this caller snapshot; malformed records, technical errors and success leave it unchanged.
 * Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
AmaStatus ama_service_record_refusal(AmaServiceState *caller, AmaStatus refusal);
/** Require distinct source/output and an available source. Publish a zero-transient service copy only after validation, preserving the resume position only when requested. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
AmaStatus ama_service_clone(const AmaServiceState *source, bool preserve_resume,
    AmaServiceState *destination);
/** Copy the immediate caller refusal as immutable data, or success for a null host caller; expose no pointer to runtime controls. Component contract: [RT-09](../../../spec/implementation/runtime.md#rt-09). */
AmaNativeServices ama_native_services_snapshot(const AmaServiceState *caller);

#endif
