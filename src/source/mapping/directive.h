/**
 * @file
 * @brief Declare original-source C line mapping with explicit output errors.
 *
 * [NAME-03](../../../spec/implementation/compiler.md#name-03)
 * Generated C diagnostics must retain original physical UTF-8 paths.
 */
#ifndef AMA_SOURCE_MAPPING_DIRECTIVE_H
#define AMA_SOURCE_MAPPING_DIRECTIVE_H

#include <stdio.h>
#include "source/text/source.h"

/* Write #line with the original physical UTF-8 path, C-escaped. Requires a
 * positive line representable by C11. Caller owns final flush/close checks. */
/** Write one original-source `#line` directive with a C11-representable line and UTF-8 filename.
 * @param[in,out] output Borrowed stream; caller checks final flush/close. Partial output may precede IO failure.
 * @return Invalid source/path, line limit, IO error or success; no source bytes change.
 * @param[in] source Borrowed live source supplying the exact physical path and line index.
 * @param[in] offset Original byte offset, including EOF, selecting the C line location.
 */
SourceStatus source_write_line_directive(FILE *output, const SourceText *source, size_t offset);

#endif
