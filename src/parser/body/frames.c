/**
 * @file
 * @brief Drive indentation-scoped branch/loop frames iteratively.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * A finite frame chain owns explicit body intervals; expanded control/result trees remain T026/T027.
 */
#include "../internal.h"

bool parser_push_frame(Parser *parser, ParseFrame **frame, AstBlock *block,
    AstNode *owner, size_t indentation, ParseFrameKind kind) {
    ParseFrame *child = parser_allocate(parser, sizeof(*child), parser_site(parser));
    if (child == NULL) {
        return false;
    }
    *child = (ParseFrame){*frame, block, owner, indentation, kind};
    *frame = child;
    return true;
}

/** Close one retained branch/loop frame or switch to its alternative. Require the matching loop close at owner indentation; every successful close decreases finite frame depth.
 */
static bool finish_frame(Parser *parser, ParseFrame **frame) {
    ParseFrame *current = *frame;
    const LayoutLine *line = parser->line;
    bool at_owner = line != NULL && line != parser->end
        && current->indentation != 0 && line->indentation == current->indentation - 1;
    if (current->owner != NULL && current->block->last != NULL) {
        current->owner->span.end = current->block->last->span.end;
    }
    if (current->kind == FRAME_BRANCH && at_owner && layout_text_is(line->code, ":")) {
        current->block = &current->owner->as.control.alternative;
        current->kind = FRAME_ALTERNATIVE;
        current->owner->span.end = line->code.end;
        parser->line = line->next;
        return true;
    }
    if (current->kind == FRAME_LOOP) {
        if (!at_owner || !layout_text_is(line->code, "}")) {
            return parser_fail(parser, parser_site(parser), "A loop requires its closing marker", EXPECT_LOOP_CLOSE);
        }
        current->owner->span.end = line->code.end;
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
        parser_skip_layout(parser);
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
