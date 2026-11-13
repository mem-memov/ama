/**
 * @file
 * @brief Declare binding recognition separately from mode/order validation and append.
 *
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08)
 * Structural text order and independent direction positions precede later interface resolution.
 */
#ifndef AMA_PARSER_BINDINGS_INTERNAL_H
#define AMA_PARSER_BINDINGS_INTERNAL_H
#include "../../internal.h"
/** Recognize marker/label/operand into an already allocated argument; literals retain physical-EOL payload while omissions retain no source/receiver.
 */
bool parser_binding(Parser *parser, AstArgument *argument);
/** Check mode, uniform naming style and input-before-output order before append. Advance only the matching direction counter; do not infer a target-specific list dispatcher.
 */
bool parser_append_argument(Parser *parser, AstNode *node, AstArgument *argument);
/** Parse an absent or exact complete declaration label before the already located bounded marker; no separator trimming is permitted. */
bool parser_binding_label(Parser *, SourceSpan, size_t, AstArgument *);
/** Parse an omitted positional slot or one exact separated operand; never turn an empty literal into an omission. */
bool parser_binding_operand(Parser *, SourceSpan, size_t, AstArgument *);
#endif
