/**
 * @file
 * @brief Build nested named-index access nodes without recursive C calls.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Represent exact source spans and suffix structure without interpreting declarations or runtime values.
 */
#include "../internal.h"

/** Select the exact offending byte or zero-width end and retain its explicit failed grammar expectation.
 */
static bool fail_access(Parser *parser, SourceSpan text, size_t cursor, ParserExpectation expected) {
    SourceSpan site = {text.source, cursor, cursor < text.end ? cursor + 1 : cursor};
    return parser_fail(parser, site, "Invalid or incomplete variable access", expected);
}

bool parser_access(Parser *parser, SourceSpan text, AstAccess **output) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = text.start;
    size_t depth = 0;
    AstAccess *root = NULL;
    AstAccess **destination = &root;
    for (;;) {
        AstAccess *access = parser_allocate(parser, sizeof(*access), text);
        if (access == NULL) {
            return false;
        }
        *destination = access;
        access->span = (SourceSpan){text.source, cursor, text.end - depth};
        if (!lex_name(text, &cursor, &access->name)) {
            return fail_access(parser, text, cursor, EXPECT_ACCESS);
        }
        if (cursor < text.end && bytes[cursor] == '&') {
            access->reference = true;
            cursor++;
            if (depth != 0) {
                return fail_access(parser, text, cursor, EXPECT_ACCESS);
            }
        }
        if (cursor == text.end || bytes[cursor] != '[') {
            break;
        }
        cursor++;
        if (cursor < text.end && bytes[cursor] == ']') {
            access->whole_list = true;
            cursor++;
            if (depth != 0) {
                return fail_access(parser, text, cursor, EXPECT_ACCESS);
            }
            break;
        }
        depth++;
        destination = &access->index;
    }
    while (depth != 0 && cursor < text.end && bytes[cursor] == ']') {
        depth--;
        cursor++;
    }
    if (depth != 0 || cursor != text.end) {
        return fail_access(parser, text, cursor, depth != 0 ? EXPECT_RIGHT_BRACKET : EXPECT_ACCESS_END);
    }
    *output = root;
    return true;
}
