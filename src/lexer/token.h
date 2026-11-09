#ifndef AMA_LEXER_TOKEN_H
#define AMA_LEXER_TOKEN_H

#include <stdint.h>
#include "common/limits.h"
#include "common/source.h"

typedef enum {
    TOKEN_IDENTIFIER, TOKEN_LITERAL, TOKEN_MODULE_PATH,
    TOKEN_PARAMETER_ARROW, TOKEN_RESULT_ARROW,
    TOKEN_QUESTION_MARK, TOKEN_COLON, TOKEN_LOOP_OPEN, TOKEN_LOOP_CLOSE, TOKEN_BREAK,
    TOKEN_CALL_MARKER, TOKEN_END_OF_FILE, TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_TOKEN_TEXT_LENGTH];
    uint32_t line_number;
    uint32_t indentation;
    int is_at_line_start;
    int has_preceding_blank_line;
    const char* source_path; /* Borrowed from the source owner. */
    uint32_t column_number;
} Token;

static inline SourceLocation token_location(Token token) {
    return (SourceLocation){token.source_path, token.line_number, token.column_number};
}

#endif
