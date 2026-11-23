/** @file
 * @brief Require explicit execution mode and namespace roots before publishing a recognized source request.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * provides no default preview/apply mode; [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * never infers the root family from a directory spelling.
 */
#include "internal.h"
ProjectSourceRequestStatus project_source_request_finish(ProjectSourceRequestWork *work) {
    if (!(work->seen & ((1u << PROJECT_SOURCE_OPTION_PREVIEW) | (1u << PROJECT_SOURCE_OPTION_APPLY)))) { return PROJECT_SOURCE_REQUEST_MODE; }
    if (!work->request->root_count) { return PROJECT_SOURCE_REQUEST_ROOTS; }
    return project_source_request_exclusions(work);
}
