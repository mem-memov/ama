/**
 * @file
 * @brief Build call mode/target structure and attach its same-level refusal alternative.
 *
 * [CALL-01](../../../spec/language/calls-and-results.md#call-01)
 * Keep caller-side target syntax and lexical handler ownership explicit; direct-result trees remain T026.
 */
#include "../internal.h"

bool parser_start_call(Parser *parser, ParseFrame **frame, AstNode *node,
    SourceSpan target, AstCallMode mode) {
    node->kind = AST_CALL;
    node->as.call.mode = mode;
    if (mode == AST_CONSTANT_READ) {
        if (!parser_path(target)) {
            return parser_fail(parser, target, "Invalid shared-constant namespace", EXPECT_NAMESPACE_PATH);
        }
        node->as.call.namespace_path = target;
    } else {
        if (!parser_access(parser, target, &node->as.call.target)) {
            return false;
        }
        if (mode == AST_RESUME_CALL && node->as.call.target->whole_list) {
            return parser_fail(parser, target, "A whole list cannot be resumed", EXPECT_INSTANCE_ACCESS);
        }
    }
    size_t indentation = parser->line->indentation;
    parser->line = parser->line->next;
    if (!parser_call_arguments(parser, node, indentation + 1)) {
        return false;
    }
    parser_skip_region(parser, indentation);
    const LayoutLine *line = parser->line;
    if (line != NULL && line != parser->end && line->indentation == indentation
        && layout_text_is(line->code, ":")) {
        if (mode == AST_CONSTANT_READ) {
            return parser_fail(parser, line->code, "Static constant reads have no acquisition alternative", EXPECT_NEXT_STATEMENT);
        }
        node->as.call.has_alternative = true;
        node->span.end = line->code.end;
        parser->line = line->next;
        return parser_push_frame(parser, frame, &node->as.call.alternative, node,
            indentation + 1, FRAME_ALTERNATIVE);
    }
    return true;
}
