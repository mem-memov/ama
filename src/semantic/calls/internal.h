#ifndef AMA_SEMANTIC_CALLS_INTERNAL_H
#define AMA_SEMANTIC_CALLS_INTERNAL_H

#include "semantic/internal.h"

bool semantic_call_field(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding);
bool semantic_binding_value(SemanticProgram *program, SemanticType *caller,
    SemanticStatement *statement, SemanticBinding *binding);

#endif
