#ifndef AMA_SEMANTIC_INTERNAL_H
#define AMA_SEMANTIC_INTERNAL_H

#include "semantic.h"

typedef bool (*SemanticVisitor)(SemanticProgram *, SemanticType *, const AstNode *, size_t, void *);
bool semantic_walk(SemanticProgram *program, SemanticType *type, AstBlock block,
    SemanticVisitor visit, void *context);
bool semantic_same_text(SourceSpan left, SourceSpan right);
bool semantic_fail(SemanticProgram *program, SourceSpan span, const char *message,
    const SemanticSymbol *related);
bool semantic_memory_failure(SemanticProgram *program);
void *semantic_allocate(SemanticProgram *program, size_t size);

bool semantic_link_types(SemanticProgram *program);
void semantic_clone_capabilities(SemanticProgram *program);
bool semantic_initializers(SemanticProgram *program, SemanticType *type);
bool semantic_statements(SemanticProgram *program, SemanticType *type);
SemanticAccess *semantic_access(SemanticProgram *program, SemanticType *type,
    const AstAccess *syntax, bool writable, const SemanticSymbol *initializing);
SemanticSource *semantic_source(SemanticProgram *program, SemanticType *type,
    const AstSource *syntax, const SemanticSymbol *initializing);
bool semantic_transfer(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, SemanticSource *source, const SemanticAccess *receiver,
    const SemanticSymbol *declaration, SemanticTransfer *transfer);
bool semantic_call(SemanticProgram *program, SemanticType *type, SemanticStatement *statement);
bool semantic_condition(SemanticProgram *program, SemanticAccess *access,
    AbiOperation capability, SourceSpan site);
bool semantic_visible(SemanticProgram *program, const SemanticSymbol *symbol,
    const SemanticSymbol *initializing, SourceSpan site);

bool semantic_eager_graph(SemanticProgram *program);
bool semantic_constant_graph(SemanticProgram *program);
GraphVertex *semantic_type_vertex(DependencyGraph *graph, SemanticType *type);
GraphVertex *semantic_constant_vertex(DependencyGraph *graph, SemanticSymbol *symbol);

#endif
