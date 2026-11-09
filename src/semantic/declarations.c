#include "semantic/semantic.h"
#include "lexer/token.h"
#include <string.h>

bool declare_module_symbols(AstModule* module, SymbolTable* symbols,
    const char* module_path, Error* error) {
    module->validated = false;
    for (AstDeclaration* declaration = module->declarations; declaration; declaration = declaration->next) {
        SymbolKind kind = declaration->value.kind == AST_PATH_VALUE ? SYMBOL_MODULE_ALIAS :
            declaration->kind == AST_PARAMETER_DECLARATION ? SYMBOL_PARAMETER : SYMBOL_RESULT;
        Token value = {.type = declaration->value.kind == AST_PATH_VALUE ? TOKEN_MODULE_PATH :
            declaration->value.kind == AST_LITERAL_VALUE ? TOKEN_LITERAL : TOKEN_IDENTIFIER,
            .line_number = declaration->value.location.line};
        memcpy(value.lexeme, declaration->value.text, sizeof(value.lexeme));
        Symbol* symbol = add_declarative_symbol(symbols, module_path, declaration->name.text, kind, value, error);
        if (!symbol) return false;
        symbol->declaration = declaration->name.location;
        declaration->name.symbol_id = symbol->symbol_id;
        if (declaration->value.kind == AST_LITERAL_VALUE)
            declaration->value.literal_value = symbol->default_value;
    }
    return true;
}
