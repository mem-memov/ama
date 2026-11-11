#include "../internal.h"

static bool fail_access(Parser *parser, SourceSpan text) {
    return parser_fail(parser, text, "Invalid variable access",
        "a Unicode name, optional reference suffix, and optional [] or [named index]");
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
        if (!parser_name(text, &cursor, &access->name)) {
            return fail_access(parser, text);
        }
        if (cursor < text.end && bytes[cursor] == '&') {
            access->reference = true;
            cursor++;
            if (depth != 0) {
                return fail_access(parser, text);
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
                return fail_access(parser, text);
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
        return fail_access(parser, text);
    }
    *output = root;
    return true;
}
