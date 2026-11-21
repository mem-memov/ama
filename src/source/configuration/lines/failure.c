/**
 * @file
 * @brief Retain the original INI error site while closing a failed line cursor.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires exact
 * source positions; [PKG-03](../../../../spec/project/packages.md#pkg-03) never
 * authorizes trimming malformed configuration into a different valid input.
 */
#include "internal.h"

bool configuration_line_failure(ConfigurationLines *lines, size_t offset,
    const char *message, const char *expected) {
    size_t length = lines->source == NULL ? 0 : source_length(lines->source);
    lines->issue = (ConfigurationIssue){
        {lines->source, offset, offset < length ? offset + 1 : offset},
        message, expected
    };
    lines->failed = true;
    return false;
}
