#ifndef AMA_COMMON_DIAGNOSTIC_H
#define AMA_COMMON_DIAGNOSTIC_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    ERROR_NONE, ERROR_INVALID, ERROR_LIMIT, ERROR_CYCLE, ERROR_IO, ERROR_MEMORY
} ErrorCode;

typedef struct {
    ErrorCode code;
    uint32_t line;
    char module[512];
    char message[4096];
} Error;

/* Initialize before a sequence of operations. A failed operation sets the error;
 * clear it before starting another sequence. compile_project does this itself. */
void error_clear(Error* error);
bool error_set(Error* error, ErrorCode code, uint32_t line, const char* fmt, ...);
void error_print(FILE* out, const Error* error);
#endif
