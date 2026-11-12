/**
 * @file
 * @brief Declare human diagnostic rendering over a borrowed output stream.
 *
 * [NAME-03](../../../spec/implementation/compiler.md#name-03)
 * Preserve original source names and physical byte coordinates; callers own stream closure.
 */
#ifndef AMA_DIAGNOSTIC_RENDER_H
#define AMA_DIAGNOSTIC_RENDER_H

#include <stdio.h>
#include "diagnostics/record/diagnostic.h"

/** Renderer result separates malformed input from partial stream/flush failure. */
typedef enum {
    DIAGNOSTIC_RENDER_OK,
    DIAGNOSTIC_RENDER_INVALID,
    DIAGNOSTIC_RENDER_IO_ERROR
} DiagnosticRenderStatus;

/* Validates before writing, preserves Unicode, escapes malformed/control bytes.
 * Includes complete intersecting source lines and half-open byte offsets.
 * Flushes but does not close the borrowed stream. No compiler heap allocation. */
/** Validate before writing, then render original names, byte locations and complete intersecting source lines.
 * @param[in,out] output Borrowed stream; flush but never close. Partial text can remain on IO failure.
 * @return Invalid record/stream, IO failure or success; compiler heap allocation is unnecessary.
 * @param[in] diagnostic Borrowed validated diagnostic and source/related views, alive through the synchronous write.
 */
DiagnosticRenderStatus diagnostic_render(FILE *output, const Diagnostic *diagnostic);

#endif
