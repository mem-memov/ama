#include "ama.h"

/* Byte-oriented lexer with line and indentation metadata. The first two
 * blank lines delimit declarations; later blank lines are code whitespace.
 * Only spaces are allowed for indentation. Comments begin with #. */

#include <errno.h>

/* ============================================================================
 * Character classification
 * ============================================================================ */

static int is_digit_byte(int ch) {
    return ch >= '0' && ch <= '9';
}

/* Bytes that may appear inside an identifier (after the first char).
 * Accepts ASCII letters/digits/underscore and all UTF-8 continuation or
 * lead bytes (>= 0x80). The latter is a pragmatic approximation of
 * "Unicode letter" — a full Unicode property check would require a table. */
static int is_ident_byte(int ch) {
    if (ch >= 'a' && ch <= 'z') return 1;
    if (ch >= 'A' && ch <= 'Z') return 1;
    if (ch >= '0' && ch <= '9') return 1;
    if (ch == '_') return 1;
    if (ch >= 0x80 && ch <= 0xFF) return 1;
    return 0;
}

/* Bytes that may appear as the first char of an identifier (same as above,
 * but digits are excluded). */
static int is_ident_start_byte(int ch) {
    if (ch >= 'a' && ch <= 'z') return 1;
    if (ch >= 'A' && ch <= 'Z') return 1;
    if (ch == '_') return 1;
    if (ch >= 0x80 && ch <= 0xFF) return 1;
    return 0;
}

/* ============================================================================
 * Byte-level input helpers
 * ============================================================================ */

/* Read one byte from the lexer's current position, respecting end_pos.
 * Returns EOF if the position is at or beyond end_pos, or if fgetc
 * returns EOF (unexpected truncation). */
static int read_byte(Lexer* L) {
    if (ftell(L->f) >= L->end_pos) return EOF;
    return fgetc(L->f);
}

/* Push back a byte. Safe only immediately after read_byte. */
static void unread_byte(Lexer* L, int ch) {
    (void)L;
    if (ch != EOF) ungetc(ch, L->f);
}

/* Read one byte, normalizing "\r\n" to "\n". A lone "\r" (not followed by
 * "\n") is a hard error. At end of section, returns EOF. */
static int read_char(Lexer* L) {
    int ch = read_byte(L);
    if (ch == EOF) return EOF;
    if (ch == '\r') {
        int next = read_byte(L);
        if (next == '\n') return '\n';
        unread_byte(L, next);
        fprintf(stderr,
                "error: stray \\r (not followed by \\n) at line %u\n",
                L->current_line);
        exit(1);
    }
    return ch;
}

int split_module_sections(FILE* f, ModuleSections* sections) {
    char* line = NULL;
    size_t cap = 0;
    ssize_t len;

    uint32_t line_num = 1;
    int empty_count = 0;
    long current_pos = 0;

    sections->params_start = 0;
    sections->params_end = -1;
    sections->returns_start = -1;
    sections->returns_end = -1;
    sections->code_start = -1;
    sections->code_end = -1;
    sections->params_line = 1;
    sections->returns_line = 0;
    sections->code_line = 0;

    while ((len = getline(&line, &cap, f)) != -1) {
        long next_pos = ftell(f);

        /* Check for tab characters; they are forbidden in the whole file. */
        for (ssize_t i = 0; i < len; i++) {
            if (line[i] == '\t') {
                fprintf(stderr,
                        "error: tab character at line %u is not allowed; "
                        "use spaces for indentation\n",
                        line_num);
                free(line);
                return 0;
            }
        }

        /* Determine if the line is blank (only spaces, no content). */
        int is_blank = 1;
        for (ssize_t i = 0; i < len; i++) {
            if (line[i] == '\n' || line[i] == '\r') break;
            if (line[i] != ' ') { is_blank = 0; break; }
        }

        if (is_blank) {
            /* Only the first two blank lines are section separators.
             * Once the code section has begun (empty_count == 2), any
             * further blank line is ordinary whitespace inside the code
             * section — legal per the grammar (§3.3) — and is ignored. */
            if (empty_count < 2) {
                empty_count++;
                if (empty_count == 1) {
                    sections->params_end = current_pos;
                    sections->returns_start = next_pos;
                    sections->returns_line = line_num + 1;
                } else if (empty_count == 2) {
                    sections->returns_end = current_pos;
                    sections->code_start = next_pos;
                    sections->code_line = line_num + 1;
                }
            }
        }

        current_pos = next_pos;
        line_num++;
    }

    free(line);

    if (empty_count < 2) {
        fprintf(stderr,
                "error: module must contain exactly two blank line "
                "separators (found %d); the file must have three sections: "
                "params, returns, code\n",
                empty_count);
        return 0;
    }

    sections->code_end = current_pos;
    return 1;
}

/* ============================================================================
 * Lexer initialization
 * ============================================================================
 *
 * Positions the lexer at the given byte range and sets its state to "start
 * of file". start_line is used as the line number of the first byte; the
 * caller should pass the line number at which the section starts (see
 * ModuleSections fields).
 */

void init_lexer(Lexer* L, FILE* f, long start_pos, long end_pos,
                uint32_t start_line) {
    L->f = f;
    L->end_pos = end_pos;
    L->current_line = start_line;
    L->is_first_on_line = 1;
    L->indent_count = 0;
    L->blank_seen = 0;
    fseek(f, start_pos, SEEK_SET);
}

/* ============================================================================
 * Token retrieval
 * ============================================================================
 *
 * Returns the next significant token from the current section. Skips
 * comments and blank lines. Updates is_after_blank based on whether a
 * truly blank line occurred since the previous token.
 *
 * The loop structure is:
 *   - Read chars until a significant one is found or EOF.
 *   - Track indent (spaces at start of line).
 *   - On '\n': if the line had no significant token yet, mark blank_seen.
 *   - On '#': skip to end of line (comment); blank_seen is NOT affected.
 *   - On a significant char: build a token and return it.
 *
 * After building a token, blank_seen is cleared and is_first_on_line is
 * reset. This is done inside the token builder, not at the top of the loop.
 */

Token get_next_token(Lexer* L) {
    for (;;) {
        int ch = read_char(L);

        if (ch == EOF) {
            Token eof = {TOK_EOF, "EOF", L->current_line,
                         L->indent_count, 1, L->blank_seen};
            return eof;
        }

        /* ----- line breaks ------------------------------------------------- */
        if (ch == '\n') {
            if (L->is_first_on_line) {
                /* The line had no significant token: it was blank. */
                L->blank_seen = 1;
            }
            L->current_line++;
            L->indent_count = 0;
            L->is_first_on_line = 1;
            continue;
        }

        /* ----- comments ---------------------------------------------------- */
        if (ch == '#') {
            while ((ch = read_char(L)) != '\n' && ch != EOF)
                ;
            /* Consume the newline (or EOF; the next read will see EOF). */
            if (ch == '\n') {
                L->current_line++;
            }
            L->indent_count = 0;
            L->is_first_on_line = 1;
            /* blank_seen is intentionally NOT reset: a comment is not a
             * blank line, but it also does not erase a preceding blank line's
             * effect. */
            continue;
        }

        /* ----- whitespace -------------------------------------------------- */
        if (ch == ' ') {
            if (L->is_first_on_line) {
                L->indent_count++;
            }
            /* Mid-line spaces are just separators between tokens. */
            continue;
        }

        /* ----- significant token ------------------------------------------ */
        Token tok;
        tok.line = L->current_line;
        tok.indent = L->indent_count;
        tok.is_bol = L->is_first_on_line;
        tok.is_after_blank = L->blank_seen;

        L->is_first_on_line = 0;
        L->blank_seen = 0;

        /* Single-character tokens. */
        switch (ch) {
        case '<': tok.type = TOK_LESS;    strcpy(tok.lexeme, "<"); return tok;
        case '>': tok.type = TOK_GREATER; strcpy(tok.lexeme, ">"); return tok;
        case '?': tok.type = TOK_QUESTION;strcpy(tok.lexeme, "?"); return tok;
        case ':': tok.type = TOK_COLON;   strcpy(tok.lexeme, ":"); return tok;
        case '{': tok.type = TOK_LBRACE;  strcpy(tok.lexeme, "{"); return tok;
        case '}': tok.type = TOK_RBRACE;  strcpy(tok.lexeme, "}"); return tok;
        case '!': tok.type = TOK_BANG;    strcpy(tok.lexeme, "!"); return tok;
        default:  break;
        }

        /* Path: '.' followed by identifier segments separated by dots. */
        if (ch == '.') {
            int n = 0;
            tok.lexeme[n++] = '.';
            for (;;) {
                int next = read_char(L);
                if (next == EOF) break;
                if (is_ident_byte(next) || next == '.') {
                    if (n >= MAX_LEXEME_LEN - 1) {
                        fprintf(stderr,
                                "error: path too long at line %u (max %d "
                                "bytes)\n", tok.line, MAX_LEXEME_LEN - 1);
                        exit(1);
                    }
                    tok.lexeme[n++] = (char)next;
                } else {
                    unread_byte(L, next);
                    break;
                }
            }
            tok.lexeme[n] = '\0';
            tok.type = TOK_PATH;
            return tok;
        }

        /* Literal: positive decimal integer. */
        if (is_digit_byte(ch)) {
            int n = 0;
            tok.lexeme[n++] = (char)ch;
            for (;;) {
                int next = read_char(L);
                if (next == EOF) break;
                if (is_digit_byte(next)) {
                    if (n >= MAX_LEXEME_LEN - 1) {
                        fprintf(stderr,
                                "error: literal too long at line %u "
                                "(max %d digits)\n",
                                tok.line, MAX_LEXEME_LEN - 1);
                        exit(1);
                    }
                    tok.lexeme[n++] = (char)next;
                } else {
                    unread_byte(L, next);
                    break;
                }
            }
            tok.lexeme[n] = '\0';
            tok.type = TOK_LITERAL;
            return tok;
        }

        /* Identifier: starts with a letter or underscore. */
        if (is_ident_start_byte(ch)) {
            int n = 0;
            tok.lexeme[n++] = (char)ch;
            for (;;) {
                int next = read_char(L);
                if (next == EOF) break;
                if (is_ident_byte(next)) {
                    if (n >= MAX_LEXEME_LEN - 1) {
                        fprintf(stderr,
                                "error: identifier too long at line %u "
                                "(max %d bytes)\n",
                                tok.line, MAX_LEXEME_LEN - 1);
                        exit(1);
                    }
                    tok.lexeme[n++] = (char)next;
                } else {
                    unread_byte(L, next);
                    break;
                }
            }
            tok.lexeme[n] = '\0';
            tok.type = TOK_IDENT;
            return tok;
        }

        /* Anything else is not a token. */
        fprintf(stderr,
                "error: unexpected byte 0x%02X ('%c') at line %u\n",
                (unsigned char)ch,
                (ch >= 0x20 && ch < 0x7F) ? ch : '?',
                tok.line);
        Token err = {TOK_ERROR, "ERROR", tok.line, tok.indent, 0, 0};
        return err;
    }
}

/* ============================================================================
 * Declaration section parsing
 * ============================================================================
 *
 * Reads pairs (name, value) from the current section and registers each in
 * the symbol table:
 *
 *   name
 *   value
 *   name
 *   value
 *   ...
 *
 * Restrictions:
 *   - name is TOK_IDENT, at indent 0, first on its line.
 *   - value is TOK_LITERAL, TOK_IDENT, or TOK_PATH, at indent 0.
 *   - If value is TOK_PATH, the symbol becomes an alias regardless of
 *     target_kind. This is the mechanism for declaring module aliases in
 *     the params section.
 *   - Empty sections are allowed (immediately EOF).
 *
 * Errors abort with exit(1). Section line numbers in error messages refer to
 * the whole file (start_line was passed to init_lexer).
 */

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
