/**
 * @file
 * @brief Classify one INI record before publishing any output.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) reserves column-zero
 * semicolons for whole comments and defines only three package header forms.
 */
#include "internal.h"

bool configuration_record_read(SourceSpan line, ConfigurationRecord *output,
    SourceIssue *issue) {
    if (!source_span_valid(line)) {
        return configuration_record_failure(issue, line, "Invalid configuration span", "an original line span");
    }
    ConfigurationRecord record = {.span = line};
    const unsigned char *bytes = source_bytes(line.source);
    if (line.start == line.end) { record.kind = CONFIGURATION_BLANK; }
    else if (bytes[line.start] == ';') { record.kind = CONFIGURATION_COMMENT; }
    else if (bytes[line.start] == '[') {
        if (!configuration_header(line, &record, issue)) { return false; }
    } else {
        size_t equal = line.start;
        while (equal < line.end && bytes[equal] != '=') { equal++; }
        if (equal < line.end) {
            if (!configuration_assignment(line, equal, &record, issue)) { return false; }
        } else {
            record.kind = CONFIGURATION_SELECTOR;
            record.value = line;
        }
    }
    *output = record;
    return true;
}
