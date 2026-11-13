/**
 * @file
 * @brief Start one ordinary root operation as an explicit parser frame.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) keeps the root
 * caller environment distinct from selected target contexts and refusal-body root calls.
 */
#include "../internal.h"
bool parser_start_call(Parser *parser, ParseFrame **frame, AstNode *node,
    SourceSpan target, AstCallMode mode) {
    if (!parser_call_target(parser, node, target, mode)) { return false; }
    size_t indentation = parser->line->indentation;
    parser->line = parser->line->next;
    return parser_push_frame(parser, frame, NULL, node, indentation + 1, FRAME_CALL);
}
