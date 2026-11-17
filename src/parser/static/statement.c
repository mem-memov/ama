/**
 * @file
 * @brief Compose static projections and constant namespaces from one expression owner.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) distinguishes
 * explicit static outputs from runtime constant transfers. Parsing preserves
 * that distinction without resolving the producer or allocating runtime fields.
 */
#include "internal.h"

/** Consume one expression; a following ordinary binding enters the existing
 * constant-read frame, otherwise explicit static outputs are required.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * chooses syntax from source structure alone. The caller owns partial nodes on
 * failure; only the ordinary frame driver may consume constant-read actions.
 */
bool parser_static_statement(Parser *parser, ParseFrame **frame, AstNode *node) {
    size_t indentation = parser->line->indentation;
    AstTypeExpression *expression;
    if (!parser_type_expression(parser, parser->line->code, indentation, &expression)) {
        return false;
    }
    const LayoutLine *line = parser->line;
    if (line != NULL && line != parser->end && line->indentation == indentation + 1) {
        node->kind = AST_CALL;
        node->as.call.mode = AST_CONSTANT_READ;
        node->as.call.namespace_expression = expression;
        node->span.end = expression->span.end;
        return parser_push_frame(parser, frame, NULL, node, indentation + 1, FRAME_CALL);
    }
    bool output = false;
    for (AstStaticBinding *binding = expression->first; binding != NULL; binding = binding->next) {
        if (binding->kind == AST_STATIC_OUTPUT) { output = true; }
    }
    if (!output) {
        return parser_fail(parser, expression->span,
            "A standalone static expression requires output actions", EXPECT_STATIC_BINDING);
    }
    node->kind = AST_STATIC_PROJECTION;
    node->as.static_projection = expression;
    node->span.end = expression->span.end;
    return true;
}
