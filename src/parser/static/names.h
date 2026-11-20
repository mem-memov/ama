/** @file
 * @brief Share the production recognizer for one complete atomic static name with checked source tools.
 * [TPL-01](../../../spec/language/templates.md#tpl-01) requires exactly three leading dots and complete adjoining name components.
 */
#ifndef AMA_PARSER_STATIC_NAMES_H
#define AMA_PARSER_STATIC_NAMES_H
#include "source/text/source.h"
/** Recognize the entire borrowed spelling without lookup, normalization or
 * recovery. Returns false for malformed/padded/bare prefixes; allocates nothing.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) makes syntax independent of symbol availability.
 */
bool parser_static_name(SourceSpan);
#endif
