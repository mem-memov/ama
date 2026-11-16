/**
 * @file
 * @brief Declare actual shared-value read and fresh-construction dependency checks.
 *
 * [CONST-02](../../../spec/language/declarations.md#const-02)
 * Dependencies describe values needed during construction, not every module/type reference.
 */
#ifndef AMA_SEMANTIC_CONSTANTS_INTERNAL_H
#define AMA_SEMANTIC_CONSTANTS_INTERNAL_H

#include "semantic/internal.h"

/** Add prerequisites for actual shared source/index reads and reject mutable acquisition inside immutable construction.
 * Component contract: [CONST-02](../../../spec/language/declarations.md#const-02). */
bool semantic_constant_reads(SemanticProgram *program, SemanticSymbol *owner,
    const SemanticSource *source);
/** Visit each reachable freshly constructed owned type once, adding its concrete initializer reads; finite visited identities terminate recursive type closure.
 * Component contract: [CONST-02](../../../spec/language/declarations.md#const-02). */
bool semantic_constant_construction(SemanticProgram *program, SemanticSymbol *constant);

#endif
