#include "semantic/internal.h"
#include <stdarg.h>
#include <stdlib.h>

bool semantic_error(SemanticContext* context, SourceLocation location, const char* format, ...) {
    error_set(context->error, ERROR_INVALID, location.line, "");
    snprintf(context->error->module, sizeof(context->error->module), "%s", context->module_path);
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(context->error->message, sizeof(context->error->message), format, arguments);
    va_end(arguments);
    return false;
}

Symbol* resolve_receiver(SemanticContext* context, const char* name, SourceLocation location) {
    Symbol* symbol = lookup_symbol(context->symbols, context->module_path, name);
    if (symbol && symbol->kind == SYMBOL_MODULE_ALIAS) {
        semantic_error(context, location, "alias '%s' cannot be used as a receiver", name);
        return NULL;
    }
    if (!symbol) symbol = add_local_variable(context->symbols, context->module_path, name, context->error);
    if (symbol && !symbol->declaration.line) symbol->declaration = location;
    return symbol;
}

bool resolve_value(SemanticContext* context, AstValue* value) {
    if (value->kind == AST_LITERAL_VALUE) {
        value->literal_value = atoi(value->text); /* Preserve the current literal ABI. */
        return true;
    }
    Symbol* symbol = lookup_symbol(context->symbols, context->module_path, value->text);
    if (value->kind != AST_NAME_VALUE || !symbol || symbol->kind == SYMBOL_MODULE_ALIAS)
        return semantic_error(context, value->location, "unknown numeric name '%s'", value->text);
    value->symbol_id = symbol->symbol_id;
    return true;
}
