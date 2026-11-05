#include "common/diagnostic.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

_Noreturn void fatal_at(const char* module, uint32_t line, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "error: [%s] (line %u) ", module, line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}
