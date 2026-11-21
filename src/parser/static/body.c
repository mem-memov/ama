/**
 * @file
 * @brief Compose static graph pairs and explicitly named producer selections.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) provides a separate
 * equation graph with no ordinary declarations or execution/control nodes.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) selects each
 * source production without resolving a module or an input's actual category.
 */
#include "internal.h"

/** A producer statement has a concrete path and at least one named output.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) forbids an implicit
 * first result and effect-only static calls. Partial syntax stays AST-owned.
 */
static bool producer(Parser *parser, AstNode *node) {
    AstTypeExpression *expression;
    if (!parser_type_expression(parser, parser->line->code, 0, PARSER_TYPE_PRODUCER, &expression)) { return false; }
    bool outputs = false;
    for (const AstStaticBinding *binding = expression->first; binding; binding = binding->next) {
        outputs |= binding->kind == AST_STATIC_OUTPUT;
    }
    if (!outputs) {
        return parser_fail(parser, expression->span,
            "A static producer use needs at least one named output selection", EXPECT_STATIC_BINDING);
    }
    node->kind = AST_STATIC_PROJECTION;
    node->as.static_projection = expression;
    node->span.end = expression->span.end;
    return true;
}

/** Consume complete top-level source statements, retaining no executable body.
 * Each iteration advances a source head or fails; nested values use explicit
 * frames under [COMP-01](../../../spec/implementation/compiler.md#comp-01).
 */
bool parser_static_body(Parser *parser, ParsedSection section, AstBlock *output) {
    parser->line = section.first;
    parser->end = section.end;
    parser_skip_region(parser, 0);
    while (parser->line != NULL && parser->line != parser->end) {
        SourceSpan text = parser->line->code;
        if (parser->line->indentation != 0 ||
            (!parser_static_name(text) && !parser_path(text))) {
            return parser_fail(parser, text,
                "A static graph needs a result pair or concrete producer path at indentation zero",
                EXPECT_STATIC_RESULT);
        }
        AstNode *node = parser_node(parser, AST_STATIC_RESULT, text);
        if (node == NULL) { return false; }
        bool parsed = parser_static_name(text) ? parser_static_result(parser, 0, node)
            : producer(parser, node);
        if (!parsed) { return false; }
        parser_append(output, node);
        parser_skip_region(parser, 0);
    }
    return true;
}
