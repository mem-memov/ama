/**
 * @file
 * @brief Construct two-line static result definitions with no runtime initializer list.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) requires the
 * expression head at its result name's indentation and children one level deeper.
 */
#include "internal.h"

/** Consume the required defining expression and preserve both original head sites.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * distinguishes a result definition from an input formal before kind resolution.
 */
bool parser_static_result(Parser *parser, size_t indentation, AstNode *node) {
    SourceSpan name = parser->line->code;
    if (!parser_static_name(name)) {
        return parser_fail(parser, name, "A static result needs one complete static name", EXPECT_STATIC_RESULT);
    }
    node->kind = AST_STATIC_RESULT;
    node->as.static_result.name = name;
    parser->line = parser->line->next;
    parser_skip_region(parser, indentation);
    if (parser->line == NULL || parser->line == parser->end
        || parser->line->indentation != indentation) {
        return parser_fail(parser, parser_site(parser),
            "A static result requires a same-level defining expression", EXPECT_STATIC_RESULT);
    }
    if (!parser_type_expression(parser, parser->line->code, indentation,
        &node->as.static_result.expression)) { return false; }
    node->span.end = node->as.static_result.expression->span.end;
    return true;
}
