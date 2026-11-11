#ifndef AMA_LOWER_CALLS_INTERNAL_H
#define AMA_LOWER_CALLS_INTERNAL_H

#include "lowering/control/internal.h"

typedef struct ReverseBinding {
    struct ReverseBinding *next;
    const SemanticBinding *binding;
} ReverseBinding;
ReverseBinding *lower_reverse_bindings(Lowering *lowering, const SemanticStatement *statement);
ControlNode *lower_outputs(Lowering *lowering, const SemanticStatement *statement,
    ReverseBinding *reverse, ControlNode *after);
ControlNode *lower_inputs(Lowering *lowering, const SemanticStatement *statement,
    ReverseBinding *reverse, ControlNode *after);
ControlNode *lower_install(Lowering *lowering, const SemanticStatement *statement,
    ControlNode *after, bool list);

#endif
