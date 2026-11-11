#ifndef AMA_DIAGNOSTIC_RENDER_INTERNAL_H
#define AMA_DIAGNOSTIC_RENDER_INTERNAL_H

#include "render.h"

bool diagnostic_write_bytes(FILE *output, const unsigned char *bytes, size_t length);
bool diagnostic_write_text(FILE *output, const char *text);
bool diagnostic_write_location(FILE *output, DiagnosticLocation location);
bool diagnostic_write_excerpt(FILE *output, DiagnosticLocation location);

#endif
