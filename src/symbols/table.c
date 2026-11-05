#include "symbols/table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SymbolTable G_Table = {0};

static void copy_string_checked(char* dst, size_t dst_size,
                                 const char* src, const char* what) {
    size_t len = strlen(src);
    if (len >= dst_size) {
        fprintf(stderr,
                "error: %s too long (%zu bytes, max %zu): '%.40s%s'\n",
                what, len, dst_size - 1, src,
                (len > 40) ? "..." : "");
        exit(1);
    }
    memcpy(dst, src, len + 1);
}

static Symbol* allocate_slot(void) {
    if (G_Table.count >= MAX_SYMBOLS) {
        fprintf(stderr,
                "error: symbol table is full (max %d entries); "
                "increase MAX_SYMBOLS in common/limits.h\n",
                MAX_SYMBOLS);
        exit(1);
    }
    Symbol* s = &G_Table.symbols[G_Table.count++];
    memset(s, 0, sizeof(*s));
    return s;
}

Symbol* lookup_symbol(const char* module_path, const char* name) {
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, module_path) == 0 &&
            strcmp(s->name, name) == 0)
            return s;
    }
    return NULL;
}

/* Common initialization keeps local and declared numeric names consistent. */
static Symbol* create_symbol(const char* module, const char* name, SymbolKind kind) {
    Symbol* symbol = allocate_slot();
    copy_string_checked(symbol->module_path, sizeof(symbol->module_path),
                        module, "module path");
    copy_string_checked(symbol->name, sizeof(symbol->name), name, "identifier");
    symbol->kind = kind;
    symbol->id = (uint32_t)(symbol - G_Table.symbols);
    return symbol;
}

Symbol* add_declarative_symbol(const char* module_path, const char* name,
                               SymbolKind kind, Token val_token) {
    if (lookup_symbol(module_path, name) != NULL) {
        fprintf(stderr,
                "error: [%s] '%s' is already declared "
                "(redefinition at line %u)\n",
                module_path, name, (unsigned)val_token.line);
        exit(1);
    }

    Symbol* s = create_symbol(module_path, name, kind);

    switch (kind) {
    case SYM_ALIAS:
        if (val_token.type != TOK_PATH) {
            fprintf(stderr,
                    "internal error: alias '%s' in '%s' must have a path "
                    "value (got token type %d)\n",
                    name, module_path, (int)val_token.type);
            exit(1);
        }
        copy_string_checked(s->path_link, sizeof(s->path_link),
                            val_token.lexeme, "alias path");
        s->id = 0;
        break;

    case SYM_PARAM:
    case SYM_RETURN:
        if (val_token.type == TOK_LITERAL) {
            s->default_int = atoi(val_token.lexeme);
        } else if (val_token.type == TOK_IDENT) {
            fprintf(stderr,
                    "error: [%s] '%s' has a variable default ('%s'); "
                    "defaults that reference other variables are not "
                    "supported yet. Use a literal or assign in code.\n",
                    module_path, name, val_token.lexeme);
            exit(1);
        } else {
            fprintf(stderr,
                    "internal error: [%s] '%s' has unexpected default "
                    "token type %d\n",
                    module_path, name, (int)val_token.type);
            exit(1);
        }
        s->id = (uint32_t)(s - G_Table.symbols);
        break;

    case SYM_VAR:
    default:
        fprintf(stderr,
                "internal error: add_declarative_symbol called with kind %d "
                "(use add_local_variable for locals)\n",
                (int)kind);
        exit(1);
    }

    return s;
}

Symbol* add_local_variable(const char* module_path, const char* name) {
    Symbol* existing = lookup_symbol(module_path, name);
    if (existing)
        return existing;

    return create_symbol(module_path, name, SYM_VAR);
}

Symbol* get_nth_symbol(const char* module_path, int nth, SymbolKind kind) {
    if (nth < 0)
        return NULL;

    int seen = 0;
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, module_path) != 0)
            continue;
        if (s->kind != kind)
            continue;
        if (seen == nth)
            return s;
        seen++;
    }
    return NULL;
}

