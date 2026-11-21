/**
 * @file
 * @brief Split the first INI equals sign without trimming or interpreting data.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires exactly one
 * ASCII space on each side and nonempty, unpadded key/value views.
 */
#include "internal.h"
#include "source/configuration/lines/internal.h"
#include "source/unicode/unicode.h"

/** Validate the first/last scalar of a nonempty validated view without scanning payload data.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) forbids padding while retaining internal value bytes.
 */
static bool boundary_space(SourceSpan text, bool last) {
    const unsigned char *bytes = source_bytes(text.source);
    size_t offset = last ? text.end - 1 : text.start;
    while (last && offset > text.start && (bytes[offset] & 0xc0) == 0x80) { offset--; }
    uint32_t scalar;
    size_t width;
    return !utf8_decode(bytes + offset, text.end - offset, &scalar, &width)
        || configuration_space(scalar);
}

bool configuration_assignment(SourceSpan line, size_t equal, ConfigurationRecord *record,
    SourceIssue *issue) {
    const unsigned char *bytes = source_bytes(line.source);
    if (equal - line.start < 2 || line.end - equal < 3
        || bytes[equal - 1] != ' ' || bytes[equal + 1] != ' ') {
        return configuration_record_failure(issue, line, "Invalid configuration assignment separator",
            "key = value with exactly one ASCII space on each side");
    }
    SourceSpan key = {line.source, line.start, equal - 1};
    SourceSpan value = {line.source, equal + 2, line.end};
    if (boundary_space(key, false) || boundary_space(key, true)
        || boundary_space(value, false) || boundary_space(value, true)) {
        return configuration_record_failure(issue, line, "Configuration assignment has padding",
            "nonempty key/value and exactly one separator space");
    }
    record->kind = CONFIGURATION_ASSIGNMENT;
    record->key = key;
    record->value = value;
    return true;
}
