/**
 * @file
 * @brief Begin an ordinary retained constant read from a simple Type namespace.
 *
 * [CONST-03](../../../spec/language/declarations.md#const-03) determines the
 * namespace before member lookup. [TPL-15](../../../spec/language/templates.md#tpl-15)
 * adds constructor bindings to declarations, not to these constant-read namespaces.
 */
#include "../internal.h"

/** Keep normal binding/handler parsing with the existing call-frame owner.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) makes this
 * namespace shape independent of declaration lookup or Type inference.
 */
bool parser_namespace_statement(Parser *parser, ParseFrame **frame, AstNode *node) {
    size_t indentation = parser->line->indentation;
    AstTypeExpression *expression;
    if (!parser_namespace_type(parser, indentation, &expression)) { return false; }
    node->kind = AST_CALL;
    node->as.call.mode = AST_CONSTANT_READ;
    node->as.call.namespace_expression = expression;
    node->span.end = expression->span.end;
    return parser_push_frame(parser, frame, NULL, node, indentation + 1, FRAME_CALL);
}
