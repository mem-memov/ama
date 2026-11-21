/**
 * @file
 * @brief Validate complete strict INI encoding before any line is published.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) rejects initial BOM,
 * malformed UTF-8, TAB and lone CR even in comments and unused entries.
 */
#include "internal.h"
#include "source/unicode/unicode.h"

bool configuration_lines_begin(ConfigurationLines *lines, const SourceText *source) {
    *lines = (ConfigurationLines){.source = source};
    if (source == NULL) {
        return configuration_line_failure(lines, 0, "Missing configuration source", "an immutable source");
    }
    const unsigned char *bytes = source_bytes(source);
    size_t length = source_length(source);
    if (length >= 3 && bytes[0] == 0xef && bytes[1] == 0xbb && bytes[2] == 0xbf) {
        return configuration_line_failure(lines, 0, "Configuration starts with a BOM", "UTF-8 without a byte-order mark");
    }
    for (size_t offset = 0; offset < length;) {
        uint32_t scalar;
        size_t width;
        if (!utf8_decode(bytes + offset, length - offset, &scalar, &width)) {
            return configuration_line_failure(lines, offset, "Malformed configuration encoding", "well-formed UTF-8");
        }
        if (scalar == '\t') {
            return configuration_line_failure(lines, offset, "Configuration contains TAB", "no tab characters");
        }
        if (scalar == '\r' && (offset + 1 == length || bytes[offset + 1] != '\n')) {
            return configuration_line_failure(lines, offset, "Configuration contains a lone CR", "LF or CRLF");
        }
        offset += width;
    }
    return true;
}
