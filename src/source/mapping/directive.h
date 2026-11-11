#ifndef AMA_SOURCE_MAPPING_DIRECTIVE_H
#define AMA_SOURCE_MAPPING_DIRECTIVE_H

#include <stdio.h>
#include "source/text/source.h"

/* Write #line with the original physical UTF-8 path, C-escaped. Requires a
 * positive line representable by C11. Caller owns final flush/close checks. */
SourceStatus source_write_line_directive(FILE *output, const SourceText *source, size_t offset);

#endif
