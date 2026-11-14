/** @file
 * @brief Extract a code-pinned immutable cause without exposing a live context view.
 * [EMBED-05](../../../spec/implementation/c-library.md#embed-05) permits
 * diagnostic extraction after terminal cleanup and after instance invalidation.
 */
#include "internal.h"
AmaLibraryResult ama_library_diagnostic(AmaLibraryService *service, AmaLibraryContext context,
    AmaStatus *output) {
    if (output == NULL || output->code != AMA_OK || output->message != NULL || output->message_length != 0) {
        return ama_library_result(AMA_LIBRARY_INVALID_REQUEST, ama_status_make(AMA_INVALID_STATE, NULL, 0));
    }
    AmaLibraryAccess access = {0};
    AmaLibraryResult result = ama_library_enter(service, context, &access);
    if (result.code == AMA_LIBRARY_OK) {
        *output = access.view.status;
        ama_library_leave(&access);
    }
    return result;
}
