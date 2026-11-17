/**
 * @file
 * @brief Construct a source-preserving static type head without resolving its name.
 *
 * [TPL-01](../../../spec/language/templates.md#tpl-01) supplies two disjoint
 * spelling classes; [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * leaves Type/Template and filesystem lookup to later stages.
 */
#include "internal.h"

/** Validate before allocating and publish a zero-child expression only on success.
 * Component contract: [TPL-01](../../../spec/language/templates.md#tpl-01)
 * rejects a bare prefix, invalid dotted components and runtime storage suffixes.
 */
bool static_expression_head(Parser *parser, SourceSpan head, AstTypeExpression **output) {
    bool placeholder = parser_static_name(head);
    if (!placeholder && !parser_path(head)) {
        return parser_fail(parser, head, "Invalid static type-expression head", EXPECT_STATIC_TYPE);
    }
    AstTypeExpression *expression = parser_allocate(parser, sizeof(*expression), head);
    if (expression == NULL) { return false; }
    expression->span = head;
    expression->head = head;
    expression->kind = placeholder ? AST_TYPE_PLACEHOLDER : AST_TYPE_MODULE_PATH;
    *output = expression;
    return true;
}
