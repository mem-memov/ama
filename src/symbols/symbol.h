#ifndef AMA_SYMBOLS_SYMBOL_H
#define AMA_SYMBOLS_SYMBOL_H

#include <stdint.h>
#include "common/limits.h"
#include "common/source.h"

typedef enum { SYMBOL_LOCAL_VARIABLE, SYMBOL_MODULE_ALIAS, SYMBOL_PARAMETER, SYMBOL_RESULT } SymbolKind;

typedef struct {
    char module_path[MAX_MODULE_PATH_LENGTH];
    char name[MAX_IDENTIFIER_LENGTH];
    SymbolKind kind;
    SourceLocation declaration;
    uint32_t symbol_id; /* Private table index, not an emitted name or an address. */
    char alias_path[MAX_FILE_PATH_LENGTH];
    int default_value;
} Symbol;

#endif
