/**
 * @file
 * @brief Close operation frames and attach a refusal body only at its owning target depth.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) keeps every
 * refusal marker on its operation and makes handler statements ordinary caller-context roots.
 */
#include "internal.h"

/** Pop one completed operation and extend its enclosing output subtree span without C recursion. */
static void complete(ParseFrame **frame) {
    ParseFrame *current = *frame;
    *frame = current->parent;
    if (*frame != NULL && (*frame)->kind == FRAME_CALL) {
        (*frame)->owner->span.end = current->owner->span.end;
    }
}
bool parser_call_finish(Parser *parser, ParseFrame **frame) {
    ParseFrame *current = *frame;
    AstNode *owner = current->owner;
    if (current->call_arguments_done) {
        complete(frame);
        return true;
    }
    if ((owner->as.call.mode == AST_RETAINED_READ || owner->as.call.mode == AST_CONSTANT_READ)
        && !owner->as.call.outputs_started) {
        return parser_fail(parser, owner->span, "A retained read needs at least one output action", EXPECT_READ_RECEIVER);
    }
    current->call_arguments_done = true;
    size_t indentation = current->indentation - 1;
    parser_skip_region(parser, indentation);
    const LayoutLine *line = parser->line;
    if (line != NULL && line != parser->end && line->indentation == indentation
        && layout_text_is(line->code, ":")) {
        if (owner->as.call.mode == AST_CONSTANT_READ) {
            return parser_fail(parser, line->code, "Static constant reads have no acquisition alternative", EXPECT_NEXT_STATEMENT);
        }
        owner->as.call.has_alternative = true;
        owner->as.call.refusal_marker = line->code;
        owner->span.end = line->code.end;
        parser->line = line->next;
        return parser_push_frame(parser, frame, &owner->as.call.alternative, owner,
            current->indentation, FRAME_ALTERNATIVE);
    }
    complete(frame);
    return true;
}
