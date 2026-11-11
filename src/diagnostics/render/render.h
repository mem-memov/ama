#ifndef AMA_DIAGNOSTIC_RENDER_H
#define AMA_DIAGNOSTIC_RENDER_H

#include <stdio.h>
#include "diagnostics/record/diagnostic.h"

typedef enum {
    DIAGNOSTIC_RENDER_OK,
    DIAGNOSTIC_RENDER_INVALID,
    DIAGNOSTIC_RENDER_IO_ERROR
} DiagnosticRenderStatus;

/* Validates before writing, preserves Unicode, escapes malformed/control bytes.
 * Includes complete intersecting source lines and half-open byte offsets.
 * Flushes but does not close the borrowed stream. No compiler heap allocation. */
DiagnosticRenderStatus diagnostic_render(FILE *output, const Diagnostic *diagnostic);

#endif
