/**
 * @file
 * @brief Parse an optional complete declaration label with exactly one marker separator.
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06) separates
 * named labels from operands without consulting the callee's declaration table.
 */
#include "internal.h"
bool parser_binding_label(Parser *parser, SourceSpan text, size_t marker, AstArgument *argument) {
    if (marker == text.start) { return true; }
    const unsigned char *bytes = source_bytes(text.source);
    if (bytes[marker - 1] != ' ') {
        return parser_fail(parser, text, "Named binding components need one space", EXPECT_NAMED_BINDING);
    }
    SourceSpan label = {text.source, text.start, marker - 1};
    if (!parser_access(parser, label, &argument->label)
        || !parser_declaration_name(argument->label)) {
        return parser_fail(parser, label, "A binding label must be a declaration name", EXPECT_DECLARATION_NAME);
    }
    return true;
}
