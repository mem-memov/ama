#ifndef AMA_SEMANTIC_SEMANTIC_H
#define AMA_SEMANTIC_SEMANTIC_H

#include "ast/ast.h"
#include "symbols/table.h"
#include "graph/graph.h"

bool declare_module_symbols(AstModule* module, SymbolTable* symbols,
    const char* module_path, Error* error);
bool validate_module_body(AstModule* module, SymbolTable* symbols,
    const Graph* graph, const char* module_path, Error* error);

#endif
