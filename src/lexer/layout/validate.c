/**
 * @file
 * @brief Reject malformed UTF-8, TAB and lone CR before allocating lines or classifying activation.
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) applies these rules
 * to drafts, comments, literals and ABI inputs without exception.
 */
#include "internal.h"
#include "source/unicode/unicode.h"
LayoutStatus layout_validate_source(SourceLayout *layout) {
    const SourceText *source = layout->source;
    const unsigned char *bytes = source_bytes(source);
    size_t length = source_length(source);
    size_t error;
    if (!utf8_validate(bytes, length, &error)) {
        LayoutStatus status = layout_failure(layout, LAYOUT_INVALID,
            (SourceSpan){source, error, error + 1}, "Source is not valid UTF-8", "well-formed UTF-8");
        layout->diagnostic.code = DIAGNOSTIC_INVALID_UTF8;
        return status;
    }
    for (size_t offset = 0; offset < length; offset++) {
        if (bytes[offset] == '\t' || (bytes[offset] == '\r'
            && (offset + 1 == length || bytes[offset + 1] != '\n'))) {
            return layout_failure(layout, LAYOUT_INVALID,
                (SourceSpan){source, offset, offset + 1},
                bytes[offset] == '\t' ? "Tabs are forbidden throughout Ama and ABI input"
                    : "A lone CR is not a line ending",
                bytes[offset] == '\t' ? "ASCII spaces" : "LF or CRLF");
        }
    }
    return LAYOUT_OK;
}
