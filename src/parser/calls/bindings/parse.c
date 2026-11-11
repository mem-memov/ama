#include "internal.h"

bool parser_binding(Parser *parser, AstArgument *argument) {
    SourceSpan text = parser->line->code;
    const unsigned char *bytes = source_bytes(text.source);
    size_t marker = text.start;
    while (marker < text.end && bytes[marker] != '<' && bytes[marker] != '>') {
        marker++;
    }
    if (marker == text.end) {
        return parser_fail(parser, text, "Expected a call binding", EXPECT_CALL_BINDING);
    }
    if (marker != text.start) {
        if (bytes[marker - 1] != ' ') {
            return parser_fail(parser, text, "Named binding components need spaces", EXPECT_NAMED_BINDING);
        }
        SourceSpan label = {text.source, text.start, marker};
        while (label.end > label.start && bytes[label.end - 1] == ' ') {
            label.end--;
        }
        if (!parser_access(parser, label, &argument->label)
            || !parser_declaration_name(argument->label)) {
            return parser_fail(parser, label, "A binding label must be a declaration name", EXPECT_DECLARATION_NAME);
        }
    }
    size_t end = marker + 1;
    if (bytes[marker] == '>' && end < text.end && bytes[end] == '>') {
        end++;
    }
    argument->kind = bytes[marker] == '<' ? AST_INPUT
        : end - marker == 2 ? AST_SAVED_OUTPUT : AST_OUTPUT;
    argument->marker = (SourceSpan){text.source, marker, end};
    if (end == text.end && argument->label == NULL && argument->kind != AST_SAVED_OUTPUT) {
        argument->omitted = true;
        return true;
    }
    if (end == text.end || bytes[end] != ' ') {
        return parser_fail(parser, text, "A binding requires a separated operand", EXPECT_BOUND_OPERAND);
    }
    while (end < text.end && bytes[end] == ' ') {
        end++;
    }
    SourceSpan operand = {text.source, end, text.end};
    return argument->kind == AST_INPUT
        ? parser_source(parser, parser->line, operand, true, &argument->source)
        : parser_access(parser, operand, &argument->receiver);
}

