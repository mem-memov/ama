#ifndef AMA_CODEGEN_AST_H
#define AMA_CODEGEN_AST_H
#include "ast/ast.h"
#include "codegen/emitter.h"

/* Only semantically validated modules may reach code generation. */
bool emit_ast_body(Emitter* emitter, const AstModule* module, Error* error);
#endif
