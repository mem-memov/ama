/**
 * @file
 * @brief Fold one finite linear header stack into a loop with one lexical repeat root.
 * [FLOW-04](../../../spec/language/control-flow.md#flow-04) forbids sibling
 * outputs and binds the repeat root to its containing ordinary body. The loop
 * owns the operation tree; each active operation frame still owns its handler.
 */
#include "../internal.h"
bool parser_condition_chain(Parser *parser, ParseFrame **frame,
    AstAccess *condition, const LayoutLine *marker) {
    if (!parser_owned_access(condition)) {
        return parser_fail(parser, condition->span, "A loop terminal requires owned scalar syntax", EXPECT_ACCESS);
    }
    ParseFrame *root = *frame;
    for (ParseFrame *link = *frame; link != NULL && link->kind == FRAME_CALL; link = link->parent) {
        AstNode *operation = link->owner;
        if (link->condition_loop != NULL || operation->as.call.namespace_path.source != NULL
            || operation->as.call.output_count != 0
            || (link == *frame && operation->as.call.first_output != NULL)
            || (link != *frame && (operation->as.call.first_output == NULL
                || operation->as.call.first_output != operation->as.call.last_output))) {
            return parser_fail(parser, operation->span, "A loop header requires one successor and no ordinary outputs", EXPECT_STATEMENT_TAIL);
        }
        root = link;
    }
    AstNode *loop = root->owner;
    AstNode *command = parser_node(parser, AST_CALL, loop->span);
    if (command == NULL) { return false; }
    command->as.call = loop->as.call;
    for (AstOutputAction *action = command->as.call.first_output; action != NULL; action = action->next) {
        action->operation->as.call.enclosing_operation = command;
    }
    AstNode *next = loop->next;
    SourceSpan span = loop->span;
    *loop = (AstNode){.next = next, .kind = AST_LOOP, .span = span};
    root->owner = command;
    loop->as.control.command = command;
    loop->as.control.condition_owner = (*frame)->owner;
    loop->as.control.condition = condition;
    loop->as.control.body_marker = marker->code;
    for (ParseFrame *link = *frame; link != NULL && link->kind == FRAME_CALL; link = link->parent) {
        link->condition_loop = loop;
        link->owner->as.call.outputs_started = true;
    }
    parser->line = marker->next;
    return parser_push_frame(parser, frame, &loop->as.control.body, loop,
        marker->indentation + 1, FRAME_LOOP);
}
