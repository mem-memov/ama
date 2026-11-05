#ifndef AMA_LEXER_TOKEN_H
#define AMA_LEXER_TOKEN_H

#include <stdint.h>
#include "common/limits.h"

typedef enum {
    TOK_IDENT, TOK_LITERAL, TOK_PATH,
    TOK_LESS, TOK_GREATER,
    TOK_QUESTION, TOK_COLON, TOK_LBRACE, TOK_RBRACE, TOK_BANG,
    TOK_EOF, TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LEN];
    uint32_t line;
    uint32_t indent;
    int is_bol;
    int is_after_blank;
} Token;

#endif
