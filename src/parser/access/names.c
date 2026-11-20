/**
 * @file
 * @brief Recognize complete Unicode names and exact Project/Standard paths without binding.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * Reuse the lexical name scanner so declaration and access words cannot receive different segmentations.
 */
#include "../internal.h"
#include "source/path/syntax.h"

bool parser_path(SourceSpan text) {
    if (!source_span_valid(text) || text.start == text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source);
    return module_path_syntax(bytes + text.start, text.end - text.start);
}

bool parser_declaration_name(const AstAccess *access) {
    return access->index == NULL;
}

bool parser_owned_access(const AstAccess *access) {
    return !access->reference && !access->whole_list;
}
