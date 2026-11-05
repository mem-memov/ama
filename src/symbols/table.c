#include "symbols/table.h"
#include <stdlib.h>
#include <string.h>

void symbols_init(SymbolTable* table) {
    /* Entries are initialized when added; resetting the count discards previous names. */
    table->count = 0;
}

Symbol* lookup_symbol(SymbolTable* table, const char* module, const char* name) {
    for (uint32_t i = 0; i < table->count; i++) {
        Symbol* symbol = &table->symbols[i];
        if (!strcmp(symbol->module_path, module) && !strcmp(symbol->name, name))
            return symbol;
    }
    return NULL;
}

static Symbol* create_symbol(SymbolTable* table, const char* module, const char* name,
                              SymbolKind kind, Error* error) {
    if (strlen(module) >= MAX_PATH_LEN || strlen(name) >= MAX_IDENT_LEN) {
        error_set(error, ERROR_LIMIT, 0, "module path or identifier too long");
        return NULL;
    }
    if (table->count == MAX_SYMBOLS) {
        error_set(error, ERROR_LIMIT, 0, "symbol table is full (max %d entries)", MAX_SYMBOLS);
        return NULL;
    }
    Symbol* symbol = &table->symbols[table->count];
    memset(symbol, 0, sizeof(*symbol));
    strcpy(symbol->module_path, module);
    strcpy(symbol->name, name);
    symbol->kind = kind;
    symbol->id = table->count++;
    return symbol;
}

static bool validate_declaration(SymbolKind kind, Token value, Error* error) {
    if (kind == SYM_ALIAS && value.type == TOK_PATH)
        return true;
    if ((kind == SYM_PARAM || kind == SYM_RETURN) && value.type == TOK_LITERAL)
        return true;
    if (value.type == TOK_IDENT)
        return error_set(error, ERROR_INVALID, value.line,
                         "defaults that reference other variables are not supported yet");
    return error_set(error, ERROR_INVALID, value.line,
                     "invalid declaration default; use a literal or an alias path");
}

Symbol* add_declarative_symbol(SymbolTable* table, const char* module, const char* name,
                               SymbolKind kind, Token value, Error* error) {
    if (lookup_symbol(table, module, name)) {
        error_set(error, ERROR_INVALID, value.line,
                  "[%s] '%s' is already declared", module, name);
        return NULL;
    }
    if (!validate_declaration(kind, value, error))
        return NULL;
    Symbol* symbol = create_symbol(table, module, name, kind, error);
    if (!symbol) return NULL;
    if (kind == SYM_ALIAS)
        strcpy(symbol->path_link, value.lexeme);
    else
        symbol->default_int = atoi(value.lexeme);
    return symbol;
}

Symbol* add_local_variable(SymbolTable* table, const char* module, const char* name,
                           Error* error) {
    Symbol* existing = lookup_symbol(table, module, name);
    return existing ? existing : create_symbol(table, module, name, SYM_VAR, error);
}

Symbol* get_nth_symbol(SymbolTable* table, const char* module, int nth, SymbolKind kind) {
    if (nth < 0) return NULL;
    for (uint32_t i = 0; i < table->count; i++) {
        Symbol* symbol = &table->symbols[i];
        if (!strcmp(symbol->module_path, module) && symbol->kind == kind && nth-- == 0)
            return symbol;
    }
    return NULL;
}
