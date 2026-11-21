/**
 * @file
 * @brief Recognize complete known INI headers and bounded repository prefixes.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) rejects unknown or padded
 * headers; [PKG-06](../../../../spec/project/packages.md#pkg-06) ends prefixes
 * before the package version and forbids aliases/masks there.
 */
#include <string.h>
#include "internal.h"

bool configuration_header(SourceSpan line, ConfigurationRecord *record, SourceIssue *issue) {
    if (configuration_text_is(line, "[dependencies]")) { record->kind = CONFIGURATION_DEPENDENCIES; }
    else if (configuration_text_is(line, "[aliases]")) { record->kind = CONFIGURATION_ALIASES; }
    else {
        static const char prefix[] = "[repository ";
        const unsigned char *bytes = source_bytes(line.source);
        size_t length = line.end - line.start;
        if (length <= sizeof(prefix) || memcmp(bytes + line.start, prefix, sizeof(prefix) - 1) != 0
            || bytes[line.end - 1] != ']') {
            return configuration_record_failure(issue, line, "Unknown configuration section",
                "[dependencies], [aliases], or [repository /prefix]");
        }
        SourceSpan name = {line.source, line.start + sizeof(prefix) - 1, line.end - 1};
        if (!configuration_repository_prefix(name)) {
            return configuration_record_failure(issue, name, "Invalid repository prefix",
                "an absolute vendor/package prefix before the version");
        }
        record->kind = CONFIGURATION_REPOSITORY;
        record->key = name;
    }
    return true;
}
