#ifndef AMA_SYMBOLS_SYMBOL_H
#define AMA_SYMBOLS_SYMBOL_H

#include <stdint.h>
#include "common/limits.h"

typedef enum { SYM_VAR, SYM_ALIAS, SYM_PARAM, SYM_RETURN } SymbolKind;

typedef struct {
    char module_path[MAX_PATH_LEN];
    char name[MAX_IDENT_LEN];
    SymbolKind kind;
    uint32_t id; /* Private ID used to name a C variable, not an address. */
    char path_link[MAX_PATH_LINK_LEN];
    int default_int;
} Symbol;

#endif
