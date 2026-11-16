/**
 * @file
 * @brief Drive indentation-scoped body and operation frames iteratively.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) warns once at each written empty marker.
 * A finite frame chain owns explicit body and operation intervals.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) requires nested
 * result operations and refusal bodies to finish without recursive C activation.
 */
#include "../internal.h"

bool parser_push_frame(Parser *parser, ParseFrame **frame, AstBlock *block,
    AstNode *owner, size_t indentation, ParseFrameKind kind) {
    ParseFrame *child = parser_allocate(parser, sizeof(*child), parser_site(parser));
    if (child == NULL) {
        return false;
    }
    *child = (ParseFrame){.parent = *frame, .block = block, .owner = owner,
        .indentation = indentation, .kind = kind};
    *frame = child;
    return true;
}

/** Close one retained branch/loop frame or switch to its alternative. Require the matching loop close at owner indentation; every successful close decreases finite frame depth.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01), [SYN-01](../../../spec/language/source-and-names.md#syn-01), [CALL-09](../../../spec/language/calls-and-results.md#call-09). */
static bool finish_frame(Parser *parser, ParseFrame **frame) {
    ParseFrame *current = *frame;
    if (current->kind == FRAME_BRANCH && current->indentation != 0) {
        parser_skip_region(parser, current->indentation - 1);
    }
    const LayoutLine *line = parser->line;
    bool at_owner = line != NULL && line != parser->end
        && current->indentation != 0 && line->indentation == current->indentation - 1;
    if (current->owner != NULL && current->block->last != NULL) {
        current->owner->span.end = current->block->last->span.end;
    }
    SourceSpan marker = {0};
    if (current->owner != NULL) {
        if (current->kind == FRAME_BRANCH || current->kind == FRAME_LOOP) {
            marker = current->owner->kind == AST_FOREACH ? current->owner->as.traversal.body_marker
                : current->owner->as.control.body_marker;
        } else if (current->kind == FRAME_ALTERNATIVE) {
            marker = current->owner->kind == AST_CALL ? current->owner->as.call.refusal_marker
                : current->owner->as.control.alternative_marker;
        }
    }
    if (!parser_empty_warning(parser, *current->block, marker)) { return false; }
    if (current->kind == FRAME_BRANCH && at_owner && layout_text_is(line->code, ":")) {
        current->block = &current->owner->as.control.alternative;
        current->kind = FRAME_ALTERNATIVE;
        current->owner->span.end = line->code.end;
        current->owner->as.control.alternative_marker = line->code;
        parser->line = line->next;
        return true;
    }
    if (current->kind == FRAME_LOOP) {
        if (!at_owner || !layout_text_is(line->code, "}")) {
            return parser_fail(parser, parser_site(parser), "A loop requires its closing marker", EXPECT_LOOP_CLOSE);
        }
        current->owner->span.end = line->code.end;
        if (current->owner->kind == AST_FOREACH) { current->owner->as.traversal.closing_marker = line->code; }
        else { current->owner->as.control.closing_marker = line->code; }
        parser->line = line->next;
    }
    *frame = current->parent;
    return true;
}

bool parser_body(Parser *parser, ParsedSection section, AstBlock *output) {
    parser->line = section.first;
    parser->end = section.end;
    ParseFrame *frame = NULL;
    if (!parser_push_frame(parser, &frame, output, NULL, 0, FRAME_ROOT)) {
        return false;
    }
    while (frame != NULL) {
        if (frame->kind == FRAME_CALL) {
            if (!parser_call_step(parser, &frame)) { return false; }
            continue;
        }
        parser_skip_region(parser, frame->indentation);
        if (parser->line == NULL || parser->line == parser->end
            || parser->line->indentation < frame->indentation) {
            if (!finish_frame(parser, &frame)) {
                return false;
            }
        } else if (parser->line->indentation != frame->indentation) {
            return parser_fail(parser, parser_site(parser), "Unexpected indentation",
                EXPECT_NESTING);
        } else if (!parser_statement(parser, &frame)) {
            return false;
        }
    }
    return true;
}
