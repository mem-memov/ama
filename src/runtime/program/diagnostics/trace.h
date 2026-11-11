#ifndef AMA_PROGRAM_TRACE_H
#define AMA_PROGRAM_TRACE_H
#include <stdio.h>
#include "runtime/program/program.h"
typedef struct AmaProgramLocation {
    uint64_t operation;
    const char *path;
    size_t line, column;
    const char *role;
    const char *type;
} AmaProgramLocation;
void ama_program_report(void *, AmaStatus);
#endif
