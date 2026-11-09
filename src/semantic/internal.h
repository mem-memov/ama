#ifndef AMA_SEMANTIC_INTERNAL_H
#define AMA_SEMANTIC_INTERNAL_H
#include "semantic/semantic.h"

typedef struct {
    SymbolTable* symbols;
    const Graph* graph;
    const char* module_path;
    Error* error;
} SemanticContext;

bool semantic_error(SemanticContext* context, SourceLocation location, const char* format, ...);
Symbol* resolve_receiver(SemanticContext* context, const char* name, SourceLocation location);
bool resolve_value(SemanticContext* context, AstValue* value);
bool validate_call(SemanticContext* context, AstStatement* statement);
#endif
