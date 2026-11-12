/**
 * @file
 * @brief Classify literal, constant, value, acquisition and continuation source syntax.
 *
 * [REF-01](../../../spec/language/values-and-references.md#ref-01)
 * Suffix/source form is structural; standalone self-reference support remains T024.
 */
#include "../internal.h"

/** Split a validated retained qualified constant at its last dot, with explicit list suffix; require both module and member without resolving either.
 */
static bool constant_source(Parser *parser, SourceSpan text, AstSource *source) {
    const unsigned char *bytes = source_bytes(text.source);
    SourceSpan path = text;
    if (path.end >= path.start + 2 && bytes[path.end - 2] == '[' && bytes[path.end - 1] == ']') {
        source->constant_list = true;
        path.end -= 2;
    }
    if (!parser_path(path)) {
        return parser_fail(parser, text, "Invalid qualified constant source", EXPECT_CONSTANT_SOURCE);
    }
    size_t last_dot = path.end;
    while (last_dot > path.start && bytes[last_dot - 1] != '.') {
        last_dot--;
    }
    if (last_dot <= path.start + 1) {
        return parser_fail(parser, text, "A constant source needs a module and member", EXPECT_QUALIFIED_CONSTANT);
    }
    source->kind = AST_CONSTANT_SOURCE;
    source->module_path = (SourceSpan){text.source, path.start, last_dot - 1};
    source->member = (SourceSpan){text.source, last_dot, path.end};
    return true;
}

bool parser_source(Parser *parser, const LayoutLine *line, SourceSpan text,
    bool literal_allowed, AstSource **output) {
    if ((line->kind == LINE_LITERAL || line->kind == LINE_INPUT_LITERAL)
        && text.start + 1 == line->payload.start) {
        text.end = line->physical.end;
    }
    if (!literal_allowed && text.start < text.end && source_bytes(text.source)[text.start] == '|') {
        return parser_fail(parser, text,
            "Literal data requires a declaration initializer or an entry input", EXPECT_COPY_SOURCE);
    }
    AstSource *source = parser_allocate(parser, sizeof(*source), text);
    if (source == NULL) {
        return false;
    }
    source->span = text;
    if (literal_allowed && (line->kind == LINE_LITERAL || line->kind == LINE_INPUT_LITERAL)
        && text.start + 1 == line->payload.start) {
        source->kind = AST_LITERAL_SOURCE;
        source->payload = line->payload;
        source->span.end = line->physical.end;
    } else if (text.start < text.end && source_bytes(text.source)[text.start] == '.') {
        if (!constant_source(parser, text, source)) {
            return false;
        }
    } else {
        unsigned char prefix = text.start < text.end ? source_bytes(text.source)[text.start] : 0;
        source->kind = prefix == '&' ? AST_ACQUIRE_SOURCE
            : prefix == '%' ? AST_CONTINUATION_SOURCE : AST_VALUE_SOURCE;
        SourceSpan operand = text;
        operand.start += prefix == '&' || prefix == '%';
        if (!parser_access(parser, operand, &source->access)) {
            return false;
        }
        if ((prefix == '&' && !parser_owned_access(source->access))
            || (prefix == '%' && source->access->whole_list)) {
            return parser_fail(parser, text, "Invalid acquisition or continuation-copy operand",
                prefix == '&' ? EXPECT_OWNED_ACCESS : EXPECT_INSTANCE_ACCESS);
        }
    }
    *output = source;
    return true;
}
