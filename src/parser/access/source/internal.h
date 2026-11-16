/**
 * @file
 * @brief Separate qualified and instance-source parsing from source-kind publication.
 * [COMP-01](../../../../spec/implementation/compiler.md#comp-01) retains syntax
 * and original views without name lookup, type inference or runtime selection.
 */
#ifndef AMA_PARSER_SOURCE_INTERNAL_H
#define AMA_PARSER_SOURCE_INTERNAL_H
#include "../../internal.h"
/** Validate a complete qualified module/member source, retaining its list marker and original component spans. Component contract: [COMP-01](../../../../spec/implementation/compiler.md#comp-01). */
bool parser_constant_source(Parser *, SourceSpan, AstSource *);
/** Validate a bounded scalar/list value or permitted reference/continuation operand and retain its exact access tree. Component contract: [COMP-01](../../../../spec/implementation/compiler.md#comp-01). */
bool parser_instance_source(Parser *, SourceSpan, AstSource *);
#endif
