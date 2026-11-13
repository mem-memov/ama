/**
 * @file
 * @brief Separate interface-field resolution from caller-side bound value checking.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) keeps parent
 * selections distinct from caller-local operands and ordered ordinary bindings.
 * Keep resolved interface ordinals distinct from binding text order and caller namespace.
 */
#ifndef AMA_SEMANTIC_CALLS_INTERNAL_H
#define AMA_SEMANTIC_CALLS_INTERNAL_H

#include "semantic/internal.h"

/** Resolve a caller root or direct parent-result target, retaining the selected parent explicitly. */
bool semantic_call_target(SemanticProgram *, SemanticType *, SemanticStatement *);
/** Resolve a role-correct interface declaration/ordinal and reject duplicate labels before value compatibility; preserve the earlier binding site.
 */
bool semantic_call_field(SemanticProgram *program, SemanticStatement *statement,
    const AstArgument *argument, SemanticBinding *binding);
/** Resolve supplied sources/receivers in caller context or preserve an explicit omission. No target body/default executes during checking.
 */
bool semantic_binding_value(SemanticProgram *program, SemanticType *caller,
    SemanticStatement *statement, SemanticBinding *binding);

#endif
