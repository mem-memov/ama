/**
 * @file
 * @brief Declare program-lifetime diagnostic locations and the synchronous report callback.
 *
 * [DEBUG-01](../../../../spec/implementation/diagnostics.md#debug-01)
 * Borrow source strings and the program; report before cleanup removes caller links and initialized storage.
 */
#ifndef AMA_PROGRAM_TRACE_H
#define AMA_PROGRAM_TRACE_H
#include <stdio.h>
#include "runtime/program/program.h"
/** Borrowed immutable generated operation location. Path/role/type strings outlive reporting; line and byte column are one-based. Component contract: [DEBUG-01](../../../../spec/implementation/diagnostics.md#debug-01). */
typedef struct AmaProgramLocation {
    uint64_t operation;
    const char *path;
    size_t line, column;
    const char *role;
    const char *type;
} AmaProgramLocation;
/** Report status bytes, then on the first error walk intact explicit caller links with private slot lookup. Later errors are secondary; call before cleanup destroys trace metadata. Component contract: [DEBUG-01](../../../../spec/implementation/diagnostics.md#debug-01). */
void ama_program_report(void *, AmaStatus);
#endif
