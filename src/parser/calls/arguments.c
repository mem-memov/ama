/**
 * @file
 * @brief Read one binding level and preserve argument spans, payloads and text order.
 *
 * [CALL-06](../../../spec/language/calls-and-results.md#call-06)
 * Positional omissions are explicit nodes, not invented default values or hidden receivers.
 */
#include "bindings/internal.h"

bool parser_call_arguments(Parser *parser, AstNode *node, size_t indentation) {
    parser_skip_layout(parser);
    while (parser->line != NULL && parser->line != parser->end
        && parser->line->indentation == indentation) {
        AstArgument *argument = parser_allocate(parser, sizeof(*argument), parser_site(parser));
        if (argument == NULL) {
            return false;
        }
        argument->span = parser->line->code;
        if (parser->line->kind == LINE_INPUT_LITERAL) {
            argument->span.end = parser->line->physical.end;
        }
        if (!parser_binding(parser, argument) || !parser_append_argument(parser, node, argument)) {
            return false;
        }
        parser->line = parser->line->next;
        parser_skip_layout(parser);
    }
    if ((node->as.call.mode == AST_RETAINED_READ || node->as.call.mode == AST_CONSTANT_READ)
        && node->as.call.output_count == 0) {
        return parser_fail(parser, node->span, "A retained read needs at least one receiver", EXPECT_READ_RECEIVER);
    }
    return true;
}
