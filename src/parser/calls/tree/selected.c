/**
 * @file
 * @brief Retain one result selector as an operation target with its explicit enclosing operation.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) does not
 * count selectors as ordinary bindings or move operands into the selected module namespace.
 */
#include "internal.h"
bool parser_selected_call(Parser *parser, ParseFrame **frame) {
    AstNode *owner = (*frame)->owner;
    if (owner->as.call.mode == AST_CONSTANT_READ) {
        return parser_fail(parser, parser_site(parser), "Static constant reads have no selected operations", EXPECT_READ_BINDINGS);
    }
    SourceSpan text = parser->line->code;
    AstNode *operation = parser_node(parser, AST_CALL, text);
    if (operation == NULL) { return false; }
    SourceSpan target;
    AstCallMode mode;
    if (!parser_call_mode(parser, text, &target, &mode)
        || !parser_call_target(parser, operation, target, mode)) { return false; }
    operation->as.call.enclosing_operation = owner;
    if (!parser_output_action(parser, owner, NULL, operation)) { return false; }
    size_t indentation = parser->line->indentation;
    parser->line = parser->line->next;
    return parser_push_frame(parser, frame, NULL, operation, indentation + 1, FRAME_CALL);
}
