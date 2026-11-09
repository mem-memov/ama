#ifndef AMA_SEMANTIC_INITIALIZERS_H
#define AMA_SEMANTIC_INITIALIZERS_H
#include "ast/ast.h"

/* Resolves identity/existence only. It does not parse literal values or create
    instances. The callback owns its context; paths/locations are borrowed.
*/
typedef bool (*AstTypeResolver)(void* context, const char* module_path,
    SourceLocation location, Error* error);
bool validate_typed_declarations(AstModule* module, AstTypeResolver resolve_type,
    void* context, Error* error);
#endif
