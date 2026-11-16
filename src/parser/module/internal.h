/**
 * @file
 * @brief Separate shared lexical activation recognition from section interval construction.
 * [SYN-04](../../../spec/language/source-and-names.md#syn-04) requires the same
 * genuine-marker recognizer for drafts and required-module parsing.
 */
#ifndef AMA_PARSER_MODULE_INTERNAL_H
#define AMA_PARSER_MODULE_INTERNAL_H
#include "sections.h"
#include "lexer/tokens/tokens.h"
/** Return a checked zero-indent standalone annotation's section ordinal, or -1 for an ordinary line. Component contract: [SYN-04](../../../spec/language/source-and-names.md#syn-04). */
int parser_annotation(const LayoutLine *);
#endif
