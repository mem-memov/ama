/**
 * @file
 * @brief Validate complete alias names and raw target syntax without expansion.
 * [PKG-04](../../../../spec/project/packages.md#pkg-04) uses ordinary tilde names,
 * allows only direct package targets and adds package-alias references for local
 * targets. Scope existence and collision checks remain separate consumers.
 */
#include "internal.h"
#include "source/path/syntax.h"
#include "source/unicode/unicode.h"

bool configuration_alias_key(SourceSpan text) {
    if (!source_span_valid(text) || text.end - text.start < 2) { return false; }
    const unsigned char *bytes = source_bytes(text.source) + text.start;
    size_t error;
    return bytes[0] == '~' && unicode_name(bytes + 1, text.end - text.start - 1, &error);
}

bool configuration_alias_target(SourceSpan text, bool allow_alias) {
    if (!source_span_valid(text) || text.start == text.end) { return false; }
    const unsigned char *bytes = source_bytes(text.source) + text.start;
    return (allow_alias || bytes[0] != '~') && module_path_syntax(bytes, text.end - text.start);
}
