#include "lexer/sections.h"
#include <stdlib.h>

static bool is_blank(const char* line) {
    for (; *line && *line != '\n' && *line != '\r'; line++)
        if (*line != ' ') return false;
    return true;
}

bool split_module_sections(FILE* file, ModuleSections* sections, Error* error) {
    *sections = (ModuleSections){.params_line = 1};
    char* line = NULL;
    size_t capacity = 0;
    ssize_t length;
    long position = 0;
    uint32_t number = 1;
    int separators = 0;
    while ((length = getline(&line, &capacity, file)) != -1) {
        long next = ftell(file);
        if (next < 0) {
            free(line);
            return error_set(error, ERROR_IO, number, "cannot locate source section");
        }
        for (ssize_t i = 0; i < length; i++) {
            if (line[i] == '\t') {
                free(line);
                return error_set(error, ERROR_INVALID, number,
                                 "tab character is not allowed; use spaces for indentation");
            }
        }
        if (separators < 2 && is_blank(line)) {
            if (separators++ == 0) {
                sections->params_end = position;
                sections->returns_start = next;
                sections->returns_line = number + 1;
            } else {
                sections->returns_end = position;
                sections->code_start = next;
                sections->code_line = number + 1;
            }
        }
        position = next;
        number++;
    }
    free(line);
    if (ferror(file))
        return error_set(error, ERROR_IO, number, "cannot read source sections");
    if (separators < 2)
        return error_set(error, ERROR_INVALID, 0,
                         "module must contain exactly two blank line separators (found %d)",
                         separators);
    sections->code_end = position;
    return true;
}
