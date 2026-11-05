#include "lexer/sections.h"
#include <stdlib.h>

int split_module_sections(FILE* f, ModuleSections* sections) {
    char* line = NULL;
    size_t cap = 0;
    ssize_t len;

    uint32_t line_num = 1;
    int empty_count = 0;
    long current_pos = 0;

    sections->params_start = 0;
    sections->params_end = -1;
    sections->returns_start = -1;
    sections->returns_end = -1;
    sections->code_start = -1;
    sections->code_end = -1;
    sections->params_line = 1;
    sections->returns_line = 0;
    sections->code_line = 0;

    while ((len = getline(&line, &cap, f)) != -1) {
        long next_pos = ftell(f);

        /* Check for tab characters; they are forbidden in the whole file. */
        for (ssize_t i = 0; i < len; i++) {
            if (line[i] == '\t') {
                fprintf(stderr,
                        "error: tab character at line %u is not allowed; "
                        "use spaces for indentation\n",
                        line_num);
                free(line);
                return 0;
            }
        }

        /* Determine if the line is blank (only spaces, no content). */
        int is_blank = 1;
        for (ssize_t i = 0; i < len; i++) {
            if (line[i] == '\n' || line[i] == '\r') break;
            if (line[i] != ' ') { is_blank = 0; break; }
        }

        if (is_blank) {
            /* Only the first two blank lines are section separators.
             * Once the code section has begun (empty_count == 2), any
             * further blank line is ordinary whitespace inside the code
             * section — legal per the grammar (§3.3) — and is ignored. */
            if (empty_count < 2) {
                empty_count++;
                if (empty_count == 1) {
                    sections->params_end = current_pos;
                    sections->returns_start = next_pos;
                    sections->returns_line = line_num + 1;
                } else if (empty_count == 2) {
                    sections->returns_end = current_pos;
                    sections->code_start = next_pos;
                    sections->code_line = line_num + 1;
                }
            }
        }

        current_pos = next_pos;
        line_num++;
    }

    free(line);

    if (empty_count < 2) {
        fprintf(stderr,
                "error: module must contain exactly two blank line "
                "separators (found %d); the file must have three sections: "
                "params, returns, code\n",
                empty_count);
        return 0;
    }

    sections->code_end = current_pos;
    return 1;
}
