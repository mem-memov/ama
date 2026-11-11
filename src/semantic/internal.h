#ifndef AMA_SEMANTIC_INTERNAL_H
#define AMA_SEMANTIC_INTERNAL_H

#include "semantic.h"

typedef bool (*SemanticVisitor)(SemanticProgram *, SemanticType *, const AstNode *, void *);
bool semantic_walk(SemanticProgram *program, SemanticType *type, AstBlock block,
    SemanticVisitor visit, void *context);
bool semantic_same_text(SourceSpan left, SourceSpan right);
bool semantic_fail(SemanticProgram *program, SourceSpan span, const char *message,
    const SemanticSymbol *related);
bool semantic_memory_failure(SemanticProgram *program);
void *semantic_allocate(SemanticProgram *program, size_t size);

#endif
