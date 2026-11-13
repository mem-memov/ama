/**
 * @file
 * @brief Parse one checked ordinary binding without advancing across its operation boundary.
 * [CALL-06](../../../spec/language/calls-and-results.md#call-06) retains explicit
 * omissions and independent direction positions; the operation frame owns sibling traversal.
 */
#include "bindings/internal.h"
bool parser_call_argument(Parser *parser, AstNode *node) {
    AstArgument *argument = parser_allocate(parser, sizeof(*argument), parser_site(parser));
    if (argument == NULL) { return false; }
    argument->span = parser->line->code;
    if (parser->line->kind == LINE_INPUT_LITERAL) { argument->span.end = parser->line->physical.end; }
    if (!parser_binding(parser, argument) || !parser_append_argument(parser, node, argument)) { return false; }
    parser->line = parser->line->next;
    return true;
}
