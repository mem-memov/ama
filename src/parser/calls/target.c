/**
 * @file
 * @brief Parse exact call-head modes and scalar/list/constant target shape without binding.
 * [CALL-01](../../../spec/language/calls-and-results.md#call-01) separates entry,
 * resume and retained reads; [SYN-06](../../../spec/language/source-and-names.md#syn-06)
 * fixes the suffix separator rather than allowing whitespace repair.
 */
#include "../internal.h"
bool parser_call_mode(Parser *parser, SourceSpan text, SourceSpan *target, AstCallMode *mode) {
    const unsigned char *bytes = source_bytes(text.source);
    *target = text;
    *mode = AST_RETAINED_READ;
    if (text.start == text.end) { return true; }
    unsigned char last = bytes[text.end - 1];
    if (last != '!' && last != '%') { return true; }
    if (text.end - text.start < 2 || bytes[text.end - 2] != ' ') {
        return parser_fail(parser, text, "An execution marker requires one preceding space", EXPECT_STATEMENT_TAIL);
    }
    target->end -= 2;
    *mode = last == '!' ? AST_ENTRY_CALL : AST_RESUME_CALL;
    return true;
}
bool parser_call_target(Parser *parser, AstNode *node, SourceSpan target, AstCallMode mode) {
    node->kind = AST_CALL;
    node->as.call.mode = mode;
    if (mode == AST_CONSTANT_READ) {
        if (!parser_path(target)) {
            return parser_fail(parser, target, "Invalid shared-constant namespace", EXPECT_NAMESPACE_PATH);
        }
        node->as.call.namespace_path = target;
        return true;
    }
    if (!parser_access(parser, target, &node->as.call.target)) { return false; }
    if (mode == AST_RESUME_CALL && node->as.call.target->whole_list) {
        return parser_fail(parser, target, "A whole list cannot be resumed", EXPECT_INSTANCE_ACCESS);
    }
    return true;
}
