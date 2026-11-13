/**
 * @file
 * @brief Parse value, reference-acquisition and continuation-copy operands without resolving their names.
 * [REF-01](../../../../spec/language/values-and-references.md#ref-01) restricts
 * acquisitions to owned accesses; [CONT-04](../../../../spec/language/continuations.md#cont-04)
 * preserves explicit scalar continuation-source syntax.
 */
#include "internal.h"
bool parser_instance_source(Parser *parser, SourceSpan text, AstSource *source) {
    unsigned char prefix = text.start < text.end ? source_bytes(text.source)[text.start] : 0;
    source->kind = prefix == '&' ? AST_ACQUIRE_SOURCE
        : prefix == '%' ? AST_CONTINUATION_SOURCE : AST_VALUE_SOURCE;
    SourceSpan operand = text;
    if (prefix == '%') {
        if (operand.end - operand.start < 2 || source_bytes(text.source)[operand.start + 1] != ' ') {
            return parser_fail(parser, text, "Continuation copying requires one following space", EXPECT_COPY_SOURCE);
        }
        operand.start += 2;
    } else if (prefix == '&') {
        operand.start++;
    }
    if (!parser_access(parser, operand, &source->access)) {
        return false;
    }
    if ((prefix == '&' && !parser_owned_access(source->access))
        || (prefix == '%' && source->access->whole_list)) {
        return parser_fail(parser, text, "Invalid acquisition or continuation-copy operand",
            prefix == '&' ? EXPECT_OWNED_ACCESS : EXPECT_INSTANCE_ACCESS);
    }
    return true;
}
