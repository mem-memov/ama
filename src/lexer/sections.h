#ifndef AMA_LEXER_SECTIONS_H
#define AMA_LEXER_SECTIONS_H

#include <stdio.h>
#include "common/diagnostic.h"
#include <stdint.h>

typedef struct {
    long parameters_start, parameters_end;
    long results_start, results_end;
    long code_start, code_end;

    /* 1-based line numbers of the first line in each section.
        * Used by init_lexer so token.line_number refers to the real file line,
        * not to a line relative to the section start. */
    uint32_t parameters_line;
    uint32_t results_line;
    uint32_t code_line;
} ModuleSections;

bool split_module_sections(FILE* source_file, ModuleSections* sections, Error* error);

#endif
