/** @file
 * @brief Parse complete UTF-8 library configuration lines without interpreting Ama source.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) makes this a
 * separate compiler input; empty/comment lines create no exports or hidden defaults.
 */
#include <assert.h>
#include <string.h>
#include "../internal.h"
#include "source/unicode/unicode.h"
bool library_configuration_parse(LibraryBuild *build, const SourceText *source) {
    assert(!build->attempted && build->configuration == NULL);
    build->configuration = source;
    const unsigned char *bytes = source_bytes(source);
    size_t length = source_length(source), invalid = 0;
    if (!utf8_validate(bytes, length, &invalid)) {
        library_fail(build, (SourceSpan){source, invalid, invalid}, "Library configuration must be valid UTF-8");
        build->error.code = DIAGNOSTIC_INVALID_UTF8;
        return false;
    }
    for (size_t start = 0; start < length;) {
        size_t end = start;
        while (end < length && bytes[end] != '\n') {
            if (bytes[end] == 0 || bytes[end] == '\r' || bytes[end] == '\t') {
                return library_fail(build, (SourceSpan){source, end, end + 1}, "Library configuration forbids NUL, CR and tab bytes");
            }
            end++;
        }
        if (start != end && bytes[start] != '#'
            && !library_configuration_record(build, (SourceSpan){source, start, end})) { return false; }
        start = end == length ? length : end + 1;
    }
    SourceSpan all = {source, 0, length};
    if (!build->format_seen || build->component == NULL || build->abi == NULL
        || build->target == NULL || build->calling_convention == NULL || build->exports == NULL) {
        build->parsed = false;
        return library_fail(build, all, "Library configuration needs its format, component, ABI, target, calling convention and explicit exports");
    }
    if (strcmp(build->abi, "ama-c-1") != 0 || strcmp(build->calling_convention, "c") != 0) {
        build->parsed = false;
        return library_fail(build, all, "Unsupported library ABI edition or calling convention");
    }
    build->parsed = true;
    return true;
}
