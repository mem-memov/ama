#include "lexer/sections.h"
#include <stdlib.h>

static bool is_blank_line(const char* line_text) {
    for (; *line_text && *line_text != '\n' && *line_text != '\r'; line_text++)
        if (*line_text != ' ') return false;
    return true;
}

bool split_module_sections(FILE* file, ModuleSections* sections, Error* error) {
    *sections = (ModuleSections){.parameters_line = 1};
    char* line_text = NULL;
    size_t capacity = 0;
    ssize_t length;
    long position = 0;
    uint32_t line_number = 1;
    int separators = 0;
    while ((length = getline(&line_text, &capacity, file)) != -1) {
        long next_line_offset = ftell(file);
        if (next_line_offset < 0) {
            free(line_text);
            return error_set(error, ERROR_IO, line_number, "cannot locate source section");
        }
        for (ssize_t index = 0; index < length; index++) {
            if (line_text[index] == '\t') {
                free(line_text);
                return error_set(error, ERROR_INVALID, line_number,
                    "tab character is not allowed; use spaces for indentation");
            }
        }
        if (separators < 2 && is_blank_line(line_text)) {
            if (separators++ == 0) {
                sections->parameters_end = position;
                sections->results_start = next_line_offset;
                sections->results_line = line_number + 1;
            } else {
                sections->results_end = position;
                sections->code_start = next_line_offset;
                sections->code_line = line_number + 1;
            }
        }
        position = next_line_offset;
        line_number++;
    }
    free(line_text);
    if (ferror(file))
        return error_set(error, ERROR_IO, line_number, "cannot read source sections");
    if (separators < 2)
        return error_set(error, ERROR_INVALID, 0,
            "module must contain exactly two blank line separators (found %d)",
            separators);
    sections->code_end = position;
    return true;
}
