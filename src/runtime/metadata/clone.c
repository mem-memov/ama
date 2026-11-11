#include "service.h"

AmaStatus ama_service_clone(const AmaServiceState *source, bool preserve_resume,
    AmaServiceState *destination) {
    if (source == destination) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    AmaStatus status = ama_service_access(source, AMA_ACCESS_READ);
    if (!ama_status_is_success(status)) {
        return status;
    }
    AmaServiceState copy = {0};
    if (preserve_resume) {
        copy.resume = source->resume;
    }
    copy.activity = copy.resume.valid ? AMA_INSTANCE_YIELDED : AMA_INSTANCE_READY;
    *destination = copy;
    return ama_status_success();
}
