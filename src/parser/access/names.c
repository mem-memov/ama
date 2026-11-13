/**
 * @file
 * @brief Recognize complete Unicode names and exact Project/Standard paths without binding.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * Reuse the lexical name scanner so declaration and access words cannot receive different segmentations.
 */
#include "../internal.h"
#include "lexer/tokens/tokens.h"

bool parser_path(SourceSpan text) {
    if (!source_span_valid(text) || text.start == text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    size_t cursor = text.start;
    if (bytes[cursor++] != '.') { return false; }
    if (cursor < text.end && bytes[cursor] == '.') { cursor++; }
    for (;;) {
        SourceSpan name;
        if (!lex_name(text, &cursor, &name)) { return false; }
        if (cursor == text.end) { return true; }
        if (bytes[cursor++] != '.') { return false; }
    }
}

bool parser_declaration_name(const AstAccess *access) {
    return access->index == NULL;
}

bool parser_owned_access(const AstAccess *access) {
    return !access->reference && !access->whole_list;
}
