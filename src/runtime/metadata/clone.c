/**
 * @file
 * @brief Copy only explicitly permitted available-instance control state.
 *
 * [COPY-03](../../../spec/language/values-and-references.md#copy-03)
 * Ordinary copies clear saved positions; explicit continuation copying retains only the resume position, never caller links or refusal history.
 */
#include "service.h"

/** Require distinct source/output and an available source. Publish a zero-transient service copy only after validation, preserving the resume position only when requested. */
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
