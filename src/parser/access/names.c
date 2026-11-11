#include "../internal.h"
#include "source/unicode/unicode.h"

bool parser_name(SourceSpan text, size_t *cursor, SourceSpan *name) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t start = *cursor;
    size_t offset = start;
    while (offset < text.end) {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes + offset, text.end - offset, &scalar, &width)
            || !(offset == start ? unicode_identifier_start(scalar)
                : unicode_identifier_continue(scalar))) {
            break;
        }
        offset += width;
    }
    if (offset == start) {
        return false;
    }
    *name = (SourceSpan){text.source, start, offset};
    *cursor = offset;
    return true;
}

bool parser_path(SourceSpan text) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = text.start;
    SourceSpan name;
    do {
        if (cursor == text.end || bytes[cursor++] != '.' || !parser_name(text, &cursor, &name)) {
            return false;
        }
    } while (cursor < text.end);
    return true;
}

bool parser_declaration_name(const AstAccess *access) {
    return access->index == NULL;
}

bool parser_owned_access(const AstAccess *access) {
    return !access->reference && !access->whole_list;
}
