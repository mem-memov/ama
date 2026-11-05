#include "lexer/lexer.h"
#include "lexer/characters.h"
#include <stdlib.h>
#include <string.h>

static int read_byte(Lexer* L) {
    if (ftell(L->f) >= L->end_pos) return EOF;
    return fgetc(L->f);
}

static void unread_byte(Lexer* L, int ch) {
    (void)L;
    if (ch != EOF) ungetc(ch, L->f);
}

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

/* Return the first significant byte, preserving blank-line/comment semantics. */
static int skip_whitespace_and_comments(Lexer* L) {
    for (;;) {
        int ch = read_char(L);
        if (ch == '\n') {
            if (L->is_first_on_line)
                L->blank_seen = 1;
            L->current_line++;
            L->indent_count = 0;
            L->is_first_on_line = 1;
        } else if (ch == '#') {
            while ((ch = read_char(L)) != '\n' && ch != EOF)
                ;
            if (ch == '\n')
                L->current_line++;
            L->indent_count = 0;
            L->is_first_on_line = 1;
        } else if (ch == ' ') {
            if (L->is_first_on_line)
                L->indent_count++;
        } else {
            return ch;
        }
    }
}

static Token begin_token(Lexer* L) {
    Token token = {0};
    token.line = L->current_line;
    token.indent = L->indent_count;
    token.is_bol = L->is_first_on_line;
    token.is_after_blank = L->blank_seen;
    L->is_first_on_line = 0;
    L->blank_seen = 0;
    return token;
}

static int read_punctuation(Token* token, int ch) {
    switch (ch) {
    case '<': token->type = TOK_LESS; break;
    case '>': token->type = TOK_GREATER; break;
    case '?': token->type = TOK_QUESTION; break;
    case ':': token->type = TOK_COLON; break;
    case '{': token->type = TOK_LBRACE; break;
    case '}': token->type = TOK_RBRACE; break;
    case '!': token->type = TOK_BANG; break;
    default: return 0;
    }
    token->lexeme[0] = (char)ch;
    token->lexeme[1] = '\0';
    return 1;
}

/* The three word forms share length handling but retain their own diagnostics. */
static int word_continues(TokenType type, int ch) {
    if (type == TOK_LITERAL)
        return is_digit_byte(ch);
    return is_ident_byte(ch) || (type == TOK_PATH && ch == '.');
}

static void word_too_long(TokenType type, uint32_t line) {
    if (type == TOK_LITERAL)
        fprintf(stderr, "error: literal too long at line %u (max %d digits)\n",
                line, MAX_LEXEME_LEN - 1);
    else
        fprintf(stderr, "error: %s too long at line %u (max %d bytes)\n",
                type == TOK_PATH ? "path" : "identifier", line, MAX_LEXEME_LEN - 1);
    exit(1);
}

static Token read_word(Lexer* L, Token token, int first, TokenType type) {
    int length = 0;
    token.type = type;
    token.lexeme[length++] = (char)first;
    for (;;) {
        int ch = read_char(L);
        if (ch == EOF)
            break;
        if (!word_continues(type, ch)) {
            unread_byte(L, ch);
            break;
        }
        if (length >= MAX_LEXEME_LEN - 1)
            word_too_long(type, token.line);
        token.lexeme[length++] = (char)ch;
    }
    token.lexeme[length] = '\0';
    return token;
}

static Token read_identifier(Lexer* L, Token token, int first) {
    return read_word(L, token, first, TOK_IDENT);
}

static Token read_literal(Lexer* L, Token token, int first) {
    return read_word(L, token, first, TOK_LITERAL);
}

static Token read_path(Lexer* L, Token token) {
    return read_word(L, token, '.', TOK_PATH);
}

Token get_next_token(Lexer* L) {
    int ch = skip_whitespace_and_comments(L);
    if (ch == EOF) {
        Token eof = {TOK_EOF, "EOF", L->current_line,
                     L->indent_count, 1, L->blank_seen};
        return eof;
    }
    Token token = begin_token(L);
    if (read_punctuation(&token, ch))
        return token;
    if (ch == '.')
        return read_path(L, token);
    if (is_digit_byte(ch))
        return read_literal(L, token, ch);
    if (is_ident_start_byte(ch))
        return read_identifier(L, token, ch);

    fprintf(stderr, "error: unexpected byte 0x%02X ('%c') at line %u\n",
            (unsigned char)ch, (ch >= 0x20 && ch < 0x7F) ? ch : '?', token.line);
    Token error = {TOK_ERROR, "ERROR", token.line, token.indent, 0, 0};
    return error;
}
