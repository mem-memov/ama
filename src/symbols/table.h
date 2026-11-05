#ifndef AMA_SYMBOLS_TABLE_H
#define AMA_SYMBOLS_TABLE_H

#include "symbols/symbol.h"
#include "lexer/token.h"

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    uint32_t count;
} SymbolTable;

extern SymbolTable G_Table;

Symbol* lookup_symbol(const char* module_path, const char* name);
Symbol* add_declarative_symbol(const char* module_path, const char* name,
                               SymbolKind kind, Token val_token);
Symbol* add_local_variable(const char* module_path, const char* name);
Symbol* get_nth_symbol(const char* module_path, int nth, SymbolKind kind);

#endif
