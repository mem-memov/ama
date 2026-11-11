#include "service.h"

AmaStatus ama_service_access(const AmaServiceState *service, AmaAccessMode mode) {
    if (mode < AMA_ACCESS_ENTRY || mode > AMA_ACCESS_READ) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    switch (service->activity) {
        case AMA_INSTANCE_RUNNING:
        case AMA_INSTANCE_WAITING:
        case AMA_INSTANCE_ACQUIRED:
            return ama_status_make(AMA_BUSY, NULL, 0);
        case AMA_INSTANCE_FREE:
            return ama_status_make(AMA_STALE_REFERENCE, NULL, 0);
        case AMA_INSTANCE_FAILED:
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        case AMA_INSTANCE_READY:
        case AMA_INSTANCE_YIELDED:
            break;
        default:
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    bool yielded = service->activity == AMA_INSTANCE_YIELDED;
    if (!ama_return_link_is_valid(service->return_link) || service->return_link.attached ||
        yielded != service->resume.valid ||
        service->resume.valid != (service->resume.selector != 0)) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    return mode == AMA_ACCESS_RESUME && !yielded
        ? ama_status_make(AMA_NO_RESUME, NULL, 0) : ama_status_success();
}
