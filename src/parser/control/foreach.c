/**
 * @file
 * @brief Parse a list collection and reference binder without treating the binder as an index.
 * [FLOW-05](../../../spec/language/lists.md#flow-05) requires a bare collection
 * identifier and scalar reference binder; visibility and list types are later checks.
 */
#include "../internal.h"
bool parser_foreach(Parser *parser, ParseFrame **frame, AstNode *node,
    const LayoutLine *marker) {
    SourceSpan text = parser->line->code;
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = text.start;
    SourceSpan collection, binder;
    if (!lex_name(text, &cursor, &collection)) {
        return parser_fail(parser, text, "A traversal collection must be an identifier", EXPECT_ACCESS);
    }
    bool references = cursor < text.end && bytes[cursor] == '&';
    if (references) { cursor++; }
    if (cursor == text.end || bytes[cursor++] != '['
        || !lex_name(text, &cursor, &binder) || cursor + 2 != text.end
        || bytes[cursor] != '&' || bytes[cursor + 1] != ']') {
        return parser_fail(parser, text, "A traversal requires a list name and scalar reference binder", EXPECT_ACCESS);
    }
    node->kind = AST_FOREACH;
    node->as.traversal.collection = collection;
    node->as.traversal.binder = binder;
    node->as.traversal.reference_elements = references;
    node->as.traversal.body_marker = marker->code;
    node->span.end = marker->code.end;
    parser->line = marker->next;
    return parser_push_frame(parser, frame, &node->as.traversal.body, node,
        marker->indentation + 1, FRAME_LOOP);
}
