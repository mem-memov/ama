#include "common/diagnostic.h"
#include <stdarg.h>
#include <string.h>

void error_clear(Error* error) {
    memset(error, 0, sizeof(*error));
}

/* Returning false lets callers propagate an error without terminating a process. */
bool error_set(Error* error, ErrorCode code, uint32_t line, const char* fmt, ...) {
    error->module[0] = '\0';
    error->code = code;
    error->line = line;
    va_list args;
    va_start(args, fmt);
    vsnprintf(error->message, sizeof(error->message), fmt, args);
    va_end(args);
    return false;
}

void error_print(FILE* out, const Error* error) {
    fputs("error: ", out);
    if (error->module[0]) fprintf(out, "[%s] ", error->module);
    fputs(error->message, out);
    if (error->line)
        fprintf(out, " (line %u)", error->line);
    fputc('\n', out);
}
