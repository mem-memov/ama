/**
 * @file
 * @brief Publish newline-free strict INI spans without trimming original data.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) separates physical lines
 * at LF/CRLF only and rejects boundary padding, including whitespace-only lines.
 */
#include "internal.h"
#include "source/unicode/unicode.h"

/** Decode one validated boundary scalar and refuse whitespace at its lead byte.
 * The begin operation establishes complete UTF-8; no repair or locale fallback
 * is needed. [PKG-03](../../../../spec/project/packages.md#pkg-03) preserves
 * internal value bytes while refusing leading/trailing whitespace.
 */
static bool unpadded(ConfigurationLines *lines, size_t offset, size_t end) {
    uint32_t scalar;
    size_t width;
    if (!utf8_decode(source_bytes(lines->source) + offset, end - offset, &scalar, &width)) {
        return configuration_line_failure(lines, offset, "Malformed line boundary", "well-formed UTF-8");
    }
    if (configuration_space(scalar)) {
        return configuration_line_failure(lines, offset, "Configuration line has boundary whitespace",
            "a column-zero line without trailing whitespace");
    }
    return true;
}

ConfigurationLineStatus configuration_line_next(ConfigurationLines *lines, SourceSpan *output) {
    if (lines->failed) { return CONFIGURATION_INVALID_LINE; }
    size_t length = source_length(lines->source);
    if (lines->offset == length) { return CONFIGURATION_END; }
    const unsigned char *bytes = source_bytes(lines->source);
    size_t start = lines->offset;
    size_t physical_end = start;
    while (physical_end < length && bytes[physical_end] != '\n') { physical_end++; }
    size_t end = physical_end;
    if (end > start && bytes[end - 1] == '\r') { end--; }
    if (end > start) {
        size_t last = end - 1;
        while (last > start && (bytes[last] & 0xc0) == 0x80) { last--; }
        if (!unpadded(lines, start, end) || !unpadded(lines, last, end)) {
            return CONFIGURATION_INVALID_LINE;
        }
    }
    lines->offset = physical_end < length ? physical_end + 1 : physical_end;
    *output = (SourceSpan){lines->source, start, end};
    return CONFIGURATION_LINE;
}
