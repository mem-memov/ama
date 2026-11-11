#ifndef AMA_RUNTIME_METADATA_SERVICE_H
#define AMA_RUNTIME_METADATA_SERVICE_H

#include "identity.h"
#include "native/abi/services.h"

typedef enum {
    AMA_INSTANCE_FREE,
    AMA_INSTANCE_READY,
    AMA_INSTANCE_RUNNING,
    AMA_INSTANCE_WAITING,
    AMA_INSTANCE_YIELDED,
    AMA_INSTANCE_ACQUIRED,
    AMA_INSTANCE_FAILED
} AmaInstanceActivity;

typedef enum { AMA_ACCESS_ENTRY, AMA_ACCESS_RESUME, AMA_ACCESS_READ } AmaAccessMode;

typedef struct {
    bool attached;
    bool host;
    AmaHandle caller;
    uint64_t continuation;
    uint64_t call_site;
} AmaReturnLink;

typedef struct {
    bool valid;
    uint64_t selector;
} AmaResumePosition;

/* Runtime-owned controls, separate from declared values and native payload. */
typedef struct {
    AmaInstanceActivity activity;
    AmaReturnLink return_link;
    AmaResumePosition resume;
    uint64_t current_operation;
    AmaStatus last_refusal;
    AmaHandle outgoing;
    AmaInstanceActivity outgoing_previous;
} AmaServiceState;

static inline bool ama_return_link_is_valid(AmaReturnLink link) {
    if (!link.attached || link.host) {
        return (!link.host || link.attached) && ama_handle_is_empty(link.caller) &&
            link.continuation == 0 && link.call_site == 0;
    }
    return link.caller.type_id != 0 && link.caller.generation != 0 &&
        link.continuation != 0 && link.call_site != 0;
}

AmaStatus ama_service_access(const AmaServiceState *service, AmaAccessMode mode);
AmaStatus ama_service_record_refusal(AmaServiceState *caller, AmaStatus refusal);
AmaStatus ama_service_clone(const AmaServiceState *source, bool preserve_resume,
    AmaServiceState *destination);
AmaNativeServices ama_native_services_snapshot(const AmaServiceState *caller);

#endif
