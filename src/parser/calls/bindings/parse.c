/**
 * @file
 * @brief Compose exact marker, optional declaration label and source/receiver operand parsing.
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06) records
 * omissions and complete labels; no type or callee-arity inference chooses syntax.
 */
#include "internal.h"
bool parser_binding(Parser *parser, AstArgument *argument) {
    SourceSpan text = parser->line->code;
    const unsigned char *bytes = source_bytes(text.source);
    size_t marker = text.start;
    while (marker < text.end && bytes[marker] != '<' && bytes[marker] != '>') { marker++; }
    if (marker == text.end) {
        return parser_fail(parser, text, "Expected a call binding", EXPECT_CALL_BINDING);
    }
    if (!parser_binding_label(parser, text, marker, argument)) { return false; }
    size_t end = marker + 1;
    if (bytes[marker] == '>' && end < text.end && bytes[end] == '>') { end++; }
    argument->kind = bytes[marker] == '<' ? AST_INPUT
        : end - marker == 2 ? AST_SAVED_OUTPUT : AST_OUTPUT;
    argument->marker = (SourceSpan){text.source, marker, end};
    return parser_binding_operand(parser, text, end, argument);
}
