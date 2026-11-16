/** @file
 * @brief Project original checked source metadata into lossless structured tool observations.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains exact byte coordinates and source declarations independently of generated C identifiers.
 */
#ifndef AMA_TOOLS_SOURCE_OUTPUT_H
#define AMA_TOOLS_SOURCE_OUTPUT_H
#include "../json/json.h"
#include "semantic/semantic.h"
/** Nullable physical/name bytes preserve invalid filesystem UTF-8 through hexadecimal transport. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_bytes(ToolJsonWriter *, const char *);
/** Emit a nullable bounded source range and one-based original line/byte column. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_span(ToolJsonWriter *, SourceSpan);
/** Emit exact bytes within a nullable source range without interpretation or trimming. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_spelling(ToolJsonWriter *, SourceSpan);
/** Emit the exact nominal logical module identity, or null when no nominal type exists. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_type(ToolJsonWriter *, const SemanticType *);
/** Emit one checked declaration's original name/type/role/storage and source position. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_symbol(ToolJsonWriter *, const SemanticSymbol *);
/** Emit diagnostic code, severity, prose and every original/related location while its owner remains alive. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_diagnostic(ToolJsonWriter *, const Diagnostic *, bool warning);
#endif
