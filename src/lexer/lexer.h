#ifndef AMA_LEXER_LEXER_H
#define AMA_LEXER_LEXER_H

#include <stdio.h>
#include "common/diagnostic.h"
#include "lexer/token.h"

typedef struct {
    Error* error;
    FILE* source_file;
    const char* source_path;
    uint32_t column_number;
    long section_end_offset;
    uint32_t line_number;
    int is_at_line_start;
    uint32_t indentation;
    int has_preceding_blank_line;
} Lexer;

bool init_lexer(Lexer* lexer, FILE* source_file, const char* source_path,
    long section_start_offset, long section_end_offset, uint32_t start_line, Error* error);
Token get_next_token(Lexer* lexer);
void destroy_token(Token* token);

#endif
