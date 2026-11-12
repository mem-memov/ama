/**
 * @file
 * @brief Apply the retained active-layout CR/tab checks independently of physical reading.
 *
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01)
 * This older partial check is not the global TAB/spacing contract; T022 replaces its remaining gaps.
 */
#include "internal.h"

LayoutStatus layout_validate(SourceLayout *layout, bool native_abi) {
    const unsigned char *bytes = source_bytes(layout->source);
    for (LayoutLine *line = layout->first; line != NULL; line = line->next) {
        for (size_t offset = line->physical.start; offset < line->physical.end; offset++) {
            bool code_tab = bytes[offset] == '\t' && (native_abi
                || (offset < line->code.end && line->kind != LINE_LITERAL)
                || offset < line->code.start);
            if (bytes[offset] == '\r' || code_tab) {
                return layout_failure(layout, LAYOUT_INVALID,
                    (SourceSpan){layout->source, offset, offset + 1},
                    bytes[offset] == '\r' ? "A lone CR is not a line ending"
                        : "Tabs are not allowed in code whitespace",
                    bytes[offset] == '\r' ? "LF or CRLF" : "ASCII spaces");
            }
        }
    }
    return LAYOUT_OK;
}
