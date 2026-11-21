/** @file
 * @brief Retain a single bare foreign export name without resolving its module.
 * [TPL-15](../../../spec/language/templates.md#tpl-15) separates named export
 * selection from input bindings and local parameter introduction.
 * [CONST-03](../../../spec/language/declarations.md#const-03) reuses this shape
 * in a simple constant namespace without adding declaration arguments.
 */
#include "internal.h"

/** Change only the validated expression and parser cursor, without allocating.
 * [TPL-15](../../../spec/language/templates.md#tpl-15) permits one childless
 * selector after inputs; actual module kind and export membership resolve later.
 */
bool static_expression_selector(Parser *parser, AstTypeExpression *expression) {
    const LayoutLine *line = parser->line;
    if (expression->kind != AST_TYPE_MODULE_PATH) {
        return parser_fail(parser, line->code,
            "An export selector follows a concrete module path", EXPECT_TYPE_LINE);
    }
    if (expression->selected_export.source != NULL) {
        return parser_fail(parser, line->code,
            "A Type expression has exactly one export selector", EXPECT_TYPE_LINE);
    }
    expression->selected_export = line->code;
    expression->span.end = line->code.end;
    parser->line = line->next;
    return true;
}
