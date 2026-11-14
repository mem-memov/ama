/** @file
 * @brief Terminate an admitted nonexecution host operation under an existing context lease.
 * [EMBED-02](../../../spec/implementation/c-library.md#embed-02) makes admitted
 * instance-construction failure terminal. [LIFE-05](../../../spec/implementation/native-interface.md#life-05)
 * reports the intact primary cause and consumes all remaining resources once.
 */
#include "internal.h"
AmaStatus ama_context_fail(const AmaContextLease *lease, AmaStatus primary) {
    AmaContext *context = ama_context_leased(lease);
    primary = ama_status_checked(primary);
    if (context == NULL || primary.code == AMA_OK) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (context->world.operation_active) { return ama_status_make(AMA_BUSY, NULL, 0); }
    primary = ama_cleanup_primary(context->status, primary);
    if (context->status.code == AMA_OK) {
        context->program.current = context->control.current;
        context->world.report(context->world.report_context, primary);
    }
    context->status = ama_context_cleanup(context, primary);
    return context->status;
}
