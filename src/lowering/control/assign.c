#include "internal.h"

ControlNode *lower_assignment(Lowering *lowering, const SemanticStatement *statement, ControlNode *after) {
    const ControlOperation full[] = {CONTROL_SELECT_RECEIVER, CONTROL_SELECT_SOURCE,
        CONTROL_PREFLIGHT_REPLACEMENT, CONTROL_PREPARE_REPLACEMENT,
        CONTROL_COMMIT_REPLACEMENT, CONTROL_RELEASE_VALUES};
    ControlNode *next = after;
    for (size_t count = sizeof(full) / sizeof(full[0]); count > 0; count--) {
        ControlOperation operation = full[count - 1];
        if (statement->transfer == TRANSFER_NOOP && operation != CONTROL_SELECT_RECEIVER
            && operation != CONTROL_SELECT_SOURCE && operation != CONTROL_RELEASE_VALUES) {
            continue;
        }
        next = lower_prepend(lowering, operation, statement, NULL, next);
        if (next == NULL) {
            return NULL;
        }
    }
    return next;
}
