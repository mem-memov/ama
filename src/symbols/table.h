#ifndef AMA_SYMBOLS_TABLE_H
#define AMA_SYMBOLS_TABLE_H
#include "symbols/symbol.h"
#include "lexer/token.h"
#include "common/diagnostic.h"

typedef struct {
    Symbol symbols[MAX_SYMBOLS];
    uint32_t count;
} SymbolTable;

void symbols_init(SymbolTable* table);
Symbol* lookup_symbol(SymbolTable* table, const char* module, const char* name);
Symbol* add_declarative_symbol(SymbolTable* table, const char* module, const char* name,
                               SymbolKind kind, Token value, Error* error);
Symbol* add_local_variable(SymbolTable* table, const char* module, const char* name,
                           Error* error);
Symbol* get_nth_symbol(SymbolTable* table, const char* module, int nth, SymbolKind kind);
#endif
