/**
 * @file
 * @brief Retain written control markers and distinct negative-only bodies.
 * [FLOW-01](../../../spec/language/control-flow.md#flow-01) selects a negative
 * body without inventing a positive marker; [FLOW-02](../../../spec/language/control-flow.md#flow-02)
 * retains source emptiness independently of optimization.
 */
#include "../internal.h"
bool parser_control(Parser *parser, ParseFrame **frame, AstNode *node,
    AstAccess *condition, const LayoutLine *marker) {
    if (!parser_owned_access(condition)) {
        return parser_fail(parser, node->span, "A condition requires owned-value syntax", EXPECT_CONDITION_ADJACENCY);
    }
    bool loop = layout_text_is(marker->code, "{");
    bool negative = layout_text_is(marker->code, ":");
    node->kind = loop ? AST_LOOP : AST_BRANCH;
    node->as.control.condition = condition;
    if (negative) { node->as.control.alternative_marker = marker->code; }
    else { node->as.control.body_marker = marker->code; }
    node->span.end = marker->code.end;
    parser->line = marker->next;
    return parser_push_frame(parser, frame, negative ? &node->as.control.alternative
        : &node->as.control.body, node, marker->indentation + 1,
        loop ? FRAME_LOOP : negative ? FRAME_ALTERNATIVE : FRAME_BRANCH);
}
