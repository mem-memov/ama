/**
 * @file
 * @brief Separate diagnostic byte escaping, locations and excerpts from record validation.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Rendering helpers borrow validated records and report IO failure without compiler heap allocation.
 */
#ifndef AMA_DIAGNOSTIC_RENDER_INTERNAL_H
#define AMA_DIAGNOSTIC_RENDER_INTERNAL_H

#include "render.h"

/** Write finite borrowed bytes preserving valid printable UTF-8 and hex-escaping malformed/control bytes. Return false on partial IO failure.
 */
bool diagnostic_write_bytes(FILE *output, const unsigned char *bytes, size_t length);
/** Render one valid NUL-terminated borrowed string using diagnostic byte escaping.
 */
bool diagnostic_write_text(FILE *output, const char *text);
/** Render a validated source/path/absent location with original one-based physical byte coordinates. Does not acquire or close output.
 */
bool diagnostic_write_location(FILE *output, DiagnosticLocation location);
/** Render every physical line intersecting a validated half-open span. Empty/EOF spans select their point line; absent source writes nothing.
 */
bool diagnostic_write_excerpt(FILE *output, DiagnosticLocation location);

#endif
