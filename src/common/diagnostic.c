#include "common/diagnostic.h"
#include <stdarg.h>
#include <string.h>

void error_clear(Error* error) {
    memset(error, 0, sizeof(*error));
}

/* Returning false lets callers propagate an error without terminating a process. */
bool error_set(Error* error, ErrorCode error_code, uint32_t line_number, const char* format, ...) {
    error->module[0] = '\0';
    error->error_code = error_code;
    error->line_number = line_number;
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(error->message, sizeof(error->message), format, arguments);
    va_end(arguments);
    return false;
}

void error_print(FILE* output, const Error* error) {
    fputs("error: ", output);
    if (error->module[0]) fprintf(output, "[%s] ", error->module);
    fputs(error->message, output);
    if (error->line_number)
        fprintf(output, " (line %u)", error->line_number);
    fputc('\n', output);
}
