/**
 * @file
 * @brief Build exact static binding labels, skips and source/receiver heads.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) separates these
 * compile-time operands from runtime input values and retained result reads.
 * No callee identity, interface category or declaration table enters this stage.
 */
#include "internal.h"

/** A static binding begins with three dots and contains a single direction
 * marker; full label/operand validation remains with the construction operation.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * distinguishes these lines from ordinary initializers and saved reads.
 */
bool static_binding_line(const LayoutLine *line) {
    if (line->kind != LINE_CODE || line->code.end - line->code.start < 3) { return false; }
    const unsigned char *bytes = source_bytes(line->code.source);
    size_t start = line->code.start;
    if (bytes[start] != '.' || bytes[start + 1] != '.' || bytes[start + 2] != '.') { return false; }
    for (size_t offset = start + 3; offset < line->code.end; offset++) {
        if (bytes[offset] == '<' || bytes[offset] == '>') { return true; }
    }
    return false;
}

/** Preserve the complete foreign label or explicit bare positional prefix.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * requires exactly one source space before the marker and full atomic names.
 */
static bool binding_label(Parser *parser, AstStaticBinding *binding, size_t marker) {
    SourceSpan text = parser->line->code;
    const unsigned char *bytes = source_bytes(text.source);
    if (marker == text.start || bytes[marker - 1] != ' ') {
        return parser_fail(parser, text, "A static binding marker needs one preceding space", EXPECT_STATIC_BINDING);
    }
    binding->label = (SourceSpan){text.source, text.start, marker - 1};
    binding->positional = layout_text_is(binding->label, "...");
    if (!binding->positional && !parser_static_name(binding->label)) {
        return parser_fail(parser, binding->label, "Invalid static binding label", EXPECT_STATIC_BINDING);
    }
    return true;
}

/** Keep a nonempty static input head or static receiver, or one positional skip.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * permits omitted operands only for bare positional markers.
 */
static bool binding_operand(Parser *parser, AstStaticBinding *binding) {
    SourceSpan text = parser->line->code;
    size_t end = binding->marker.end;
    if (end == text.end && binding->positional) {
        binding->omitted = true;
        return true;
    }
    const unsigned char *bytes = source_bytes(text.source);
    if (end >= text.end || bytes[end] != ' ' || end + 1 == text.end) {
        return parser_fail(parser, text, "A named static binding needs its separated operand", EXPECT_STATIC_BINDING);
    }
    SourceSpan operand = {text.source, end + 1, text.end};
    if (binding->kind == AST_STATIC_INPUT) {
        return static_expression_head(parser, operand, &binding->input);
    }
    if (!parser_static_name(operand)) {
        return parser_fail(parser, operand, "A static output receiver must be a complete static name", EXPECT_STATIC_BINDING);
    }
    binding->receiver = operand;
    return true;
}

/** Build the complete current-line head without advancing the parser cursor.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * keeps one direction marker and source-ordered syntax independently of arity.
 */
bool static_binding_head(Parser *parser, AstStaticBinding **output) {
    SourceSpan text = parser->line->code;
    const unsigned char *bytes = source_bytes(text.source);
    size_t marker = text.start;
    while (marker < text.end && bytes[marker] != '<' && bytes[marker] != '>') { marker++; }
    if (marker == text.end) {
        return parser_fail(parser, text, "Missing static binding direction", EXPECT_STATIC_BINDING);
    }
    AstStaticBinding *binding = parser_allocate(parser, sizeof(*binding), text);
    if (binding == NULL || !binding_label(parser, binding, marker)) { return false; }
    binding->span = text;
    binding->marker = (SourceSpan){text.source, marker, marker + 1};
    binding->kind = bytes[marker] == '<' ? AST_STATIC_INPUT : AST_STATIC_OUTPUT;
    if (!binding_operand(parser, binding)) { return false; }
    *output = binding;
    return true;
}
