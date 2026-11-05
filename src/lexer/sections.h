#ifndef AMA_LEXER_SECTIONS_H
#define AMA_LEXER_SECTIONS_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    long params_start, params_end;
    long returns_start, returns_end;
    long code_start, code_end;

    /* 1-based line numbers of the first line in each section.
     * Used by init_lexer so token.line refers to the real file line,
     * not to a line relative to the section start. */
    uint32_t params_line;
    uint32_t returns_line;
    uint32_t code_line;
} ModuleSections;

int split_module_sections(FILE* f, ModuleSections* sections);

#endif
