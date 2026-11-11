#ifndef AMA_SEMANTIC_CONSTANTS_INTERNAL_H
#define AMA_SEMANTIC_CONSTANTS_INTERNAL_H

#include "semantic/internal.h"

bool semantic_constant_reads(SemanticProgram *program, SemanticSymbol *owner,
    const SemanticSource *source);
bool semantic_constant_construction(SemanticProgram *program, SemanticSymbol *constant);

#endif
