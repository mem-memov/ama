#include "symbols/table.h"
#include <stdlib.h>
#include <string.h>

void symbols_init(SymbolTable* table) {
    /* Entries are initialized when added; resetting the count discards previous names. */
    table->symbol_count = 0;
}

Symbol* lookup_symbol(SymbolTable* table, const char* module_path, const char* name) {
    for (uint32_t index = 0; index < table->symbol_count; index++) {
        Symbol* symbol = &table->symbols[index];
        if (!strcmp(symbol->module_path, module_path) && !strcmp(symbol->name, name))
            return symbol;
    }
    return NULL;
}

static Symbol* create_symbol(SymbolTable* table, const char* module_path, const char* name,
    SymbolKind kind, Error* error) {
    if (strlen(module_path) >= MAX_MODULE_PATH_LENGTH || strlen(name) >= MAX_IDENTIFIER_LENGTH) {
        error_set(error, ERROR_LIMIT, 0, "module path or identifier too long");
        return NULL;
    }
    if (table->symbol_count == MAX_SYMBOLS) {
        error_set(error, ERROR_LIMIT, 0, "symbol table is full (max %d entries)", MAX_SYMBOLS);
        return NULL;
    }
    Symbol* symbol = &table->symbols[table->symbol_count];
    memset(symbol, 0, sizeof(*symbol));
    strcpy(symbol->module_path, module_path);
    strcpy(symbol->name, name);
    symbol->kind = kind;
    symbol->symbol_id = table->symbol_count++;
    return symbol;
}

static bool validate_declaration(SymbolKind kind, Token value, Error* error) {
    if (kind == SYMBOL_MODULE_ALIAS && value.type == TOKEN_MODULE_PATH)
        return true;
    if ((kind == SYMBOL_PARAMETER || kind == SYMBOL_RESULT) && value.type == TOKEN_LITERAL)
        return true;
    if (value.type == TOKEN_IDENTIFIER)
        return error_set(error, ERROR_INVALID, value.line_number,
            "defaults that reference other variables are not supported yet");
    return error_set(error, ERROR_INVALID, value.line_number,
        "invalid declaration default; use a literal or an alias path");
}

Symbol* add_declarative_symbol(SymbolTable* table, const char* module_path, const char* name,
    SymbolKind kind, Token value, Error* error) {
    if (lookup_symbol(table, module_path, name)) {
        error_set(error, ERROR_INVALID, value.line_number,
            "[%s] '%s' is already declared", module_path, name);
        return NULL;
    }
    if (!validate_declaration(kind, value, error))
        return NULL;
    Symbol* symbol = create_symbol(table, module_path, name, kind, error);
    if (!symbol) return NULL;
    if (kind == SYMBOL_MODULE_ALIAS)
        strcpy(symbol->alias_path, value.lexeme);
    else
        symbol->default_value = atoi(value.lexeme);
    return symbol;
}

Symbol* add_local_variable(SymbolTable* table, const char* module_path, const char* name,
    Error* error) {
    Symbol* existing = lookup_symbol(table, module_path, name);
    return existing ? existing : create_symbol(table, module_path, name, SYMBOL_LOCAL_VARIABLE, error);
}

Symbol* find_symbol_by_position(SymbolTable* table, const char* module_path, int position, SymbolKind kind) {
    if (position < 0) return NULL;
    for (uint32_t index = 0; index < table->symbol_count; index++) {
        Symbol* symbol = &table->symbols[index];
        if (!strcmp(symbol->module_path, module_path) && symbol->kind == kind && position-- == 0)
            return symbol;
    }
    return NULL;
}
