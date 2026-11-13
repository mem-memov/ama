/**
 * @file
 * @brief Distinguish positional omissions from nonempty source/receiver operands.
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06) permits empty
 * positional input/output slots; [CALL-07](../../../../spec/language/calls-and-results.md#call-07)
 * keeps an empty literal distinct from an omitted input.
 */
#include "internal.h"
bool parser_binding_operand(Parser *parser, SourceSpan text, size_t end, AstArgument *argument) {
    if (end == text.end && argument->label == NULL && argument->kind != AST_SAVED_OUTPUT) {
        argument->omitted = true;
        return true;
    }
    if (end == text.end || source_bytes(text.source)[end] != ' ') {
        return parser_fail(parser, text, "A binding requires one separated operand", EXPECT_BOUND_OPERAND);
    }
    SourceSpan operand = {text.source, end + 1, text.end};
    return argument->kind == AST_INPUT
        ? parser_source(parser, parser->line, operand, true, &argument->source)
        : parser_access(parser, operand, &argument->receiver);
}
