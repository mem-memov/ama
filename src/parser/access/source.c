/**
 * @file
 * @brief Classify literal, constant, value, acquisition and continuation source syntax.
 *
 * [REF-01](../../../spec/language/values-and-references.md#ref-01)
 * Standalone self-reference has its own source tag and never fabricates an access or declaration name.
 */
#include "source/internal.h"

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
    if (text.end - text.start == 1 && source_bytes(text.source)[text.start] == '&') {
        source->kind = AST_SELF_REFERENCE_SOURCE;
        *output = source;
        return true;
    }
    if (literal_allowed && (line->kind == LINE_LITERAL || line->kind == LINE_INPUT_LITERAL)
        && text.start + 1 == line->payload.start) {
        source->kind = AST_LITERAL_SOURCE;
        source->payload = line->payload;
        source->span.end = line->physical.end;
    } else if (text.start < text.end && (source_bytes(text.source)[text.start] == '/'
        || source_bytes(text.source)[text.start] == '~'
        || source_bytes(text.source)[text.start] == '.')) {
        if (!parser_constant_source(parser, text, source)) {
            return false;
        }
    } else if (!parser_instance_source(parser, text, source)) {
        return false;
    }
    *output = source;
    return true;
}
