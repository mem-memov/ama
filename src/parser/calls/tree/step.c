/**
 * @file
 * @brief Advance one explicit operation frame through ordered bindings, selected children or completion.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) requires
 * exact source-order trees without recursive C activation or name-dependent syntax classification.
 */
#include "internal.h"

/** Angle markers cannot occur in an access name; literal payload is outside the checked code span. */
static bool binding(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    for (size_t offset = text.start; offset < text.end; offset++) {
        if (bytes[offset] == '<' || bytes[offset] == '>') { return true; }
    }
    return false;
}
bool parser_call_step(Parser *parser, ParseFrame **frame) {
    ParseFrame *current = *frame;
    if (current->call_arguments_done) { return parser_call_finish(parser, frame); }
    parser_skip_region(parser, current->indentation);
    const LayoutLine *line = parser->line;
    if (line == NULL || line == parser->end || line->indentation < current->indentation) {
        return parser_call_finish(parser, frame);
    }
    if (line->indentation != current->indentation) {
        return parser_fail(parser, parser_site(parser), "Unexpected operation indentation", EXPECT_NESTING);
    }
    if (current->condition_loop != NULL) {
        return parser_fail(parser, line->code, "A completed loop header has no sibling actions", EXPECT_NEXT_STATEMENT);
    }
    const LayoutLine *marker = parser_condition_marker(line->next, parser->end, line->indentation);
    if (marker != NULL && layout_text_is(marker->code, "{")) {
        AstAccess *condition;
        return parser_access(parser, line->code, &condition)
            && parser_condition_chain(parser, frame, condition, marker);
    }
    return binding(line->code) ? parser_call_argument(parser, current->owner)
        : parser_selected_call(parser, frame);
}
