#ifndef AMA_LEXER_SECTIONS_H
#define AMA_LEXER_SECTIONS_H

#include <stdio.h>
#include "common/diagnostic.h"
#include <stdint.h>

typedef struct {
    long parameters_start, parameters_end;
    long results_start, results_end;
    long code_start, code_end;

    /* Ranges exclude annotation lines. Absent declaration sections have empty
        ranges; line numbers refer to the original source file. */
    uint32_t parameters_line;
    uint32_t results_line;
    uint32_t code_line;
} ModuleSections;

bool split_module_sections(FILE* source_file, ModuleSections* sections, Error* error);

#endif
