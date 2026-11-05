#ifndef AMA_LEXER_LEXER_H
#define AMA_LEXER_LEXER_H

#include <stdio.h>
#include "lexer/token.h"

typedef struct {
    FILE* f;
    long end_pos;
    uint32_t current_line;
    int is_first_on_line;
    uint32_t indent_count;
    int blank_seen;
} Lexer;

void  init_lexer(Lexer* L, FILE* f, long start_pos, long end_pos,
                 uint32_t start_line);
Token get_next_token(Lexer* L);

#endif
