/**
 * @file
 * @brief Parse a simple ordinary Type head and its sole optional named export.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) keeps complex static
 * expressions in graphs; an ordinary field/namespace selects a ready Type.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) records the
 * foreign export name without looking it up or introducing a local parameter.
 */
#include "../static/internal.h"

/** Consume one head and at most one complete child selector, never an argument.
 * The caller consumes ordinary initializers or constant-read bindings next.
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) requires a concrete
 * path before an export selector; placeholders already denote ordinary Types.
 */
bool parser_runtime_type(Parser *parser, size_t indentation, AstTypeExpression **output) {
    AstTypeExpression *expression;
    if (!static_expression_head(parser, parser->line->code, &expression)) { return false; }
    parser->line = parser->line->next;
    parser_skip_region(parser, indentation + 1);
    const LayoutLine *line = parser->line;
    if (line != NULL && line != parser->end && line->indentation == indentation + 1
        && parser_static_name(line->code)) {
        if (expression->kind != AST_TYPE_MODULE_PATH) {
            return parser_fail(parser, line->code,
                "An export selector follows a concrete module path", EXPECT_TYPE_LINE);
        }
        expression->selected_export = line->code;
        expression->span.end = line->code.end;
        parser->line = line->next;
        parser_skip_region(parser, indentation + 1);
        line = parser->line;
        if (line != NULL && line != parser->end && line->indentation == indentation + 1
            && parser_static_name(line->code)) {
            return parser_fail(parser, line->code,
                "An ordinary Type has exactly one export selector", EXPECT_TYPE_LINE);
        }
    }
    *output = expression;
    return true;
}
