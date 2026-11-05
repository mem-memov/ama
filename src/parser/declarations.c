#include "parser/parser.h"
#include "symbols/table.h"
#include <stdlib.h>

void parse_declaration_section(Lexer* L, const char* module_path,
                                SymbolKind target_kind) {
    for (;;) {
        Token name_tok = get_next_token(L);
        if (name_tok.type == TOK_EOF) return;
        if (name_tok.type == TOK_ERROR) {
            fprintf(stderr,
                    "error: [%s] lexical error in declaration at line %u\n",
                    module_path, name_tok.line);
            exit(1);
        }

        if (name_tok.type != TOK_IDENT || !name_tok.is_bol
                || name_tok.indent != 0) {
            fprintf(stderr,
                    "error: [%s] expected a variable name at line %u, "
                    "got token type %d (declarations must start at "
                    "indent 0)\n",
                    module_path, name_tok.line, (int)name_tok.type);
            exit(1);
        }

        Token val_tok = get_next_token(L);
        if (val_tok.type == TOK_EOF) {
            fprintf(stderr,
                    "error: [%s] variable '%s' at line %u has no value; "
                    "each name must be followed by a value on the next "
                    "line\n",
                    module_path, name_tok.lexeme, name_tok.line);
            exit(1);
        }
        if (!val_tok.is_bol || val_tok.indent != 0) {
            fprintf(stderr,
                    "error: [%s] value for '%s' must be at indent 0 "
                    "(line %u)\n",
                    module_path, name_tok.lexeme, val_tok.line);
            exit(1);
        }
        if (val_tok.type != TOK_LITERAL
                && val_tok.type != TOK_IDENT
                && val_tok.type != TOK_PATH) {
            fprintf(stderr,
                    "error: [%s] invalid value for '%s' at line %u "
                    "(expected literal, identifier, or path; got token "
                    "type %d)\n",
                    module_path, name_tok.lexeme, val_tok.line,
                    (int)val_tok.type);
            exit(1);
        }

        SymbolKind final_kind =
            (val_tok.type == TOK_PATH) ? SYM_ALIAS : target_kind;

        add_declarative_symbol(module_path, name_tok.lexeme,
                               final_kind, val_tok);
    }
}
