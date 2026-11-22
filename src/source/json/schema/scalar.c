/** @file
 * @brief Extract NUL-free strings and checked canonical offset integers.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares exact data validation without sharing compiler or mutation owners;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * lossless bytes and exact integer offsets without permissive conversion;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * complete/failed decoded payloads under the caller's operation arena.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
SourceJsonStatus source_json_schema_cstring(const SourceJsonValue *value, const char **output, SourceJsonIssue *issue) {
    if (!issue || !output || !value || value->kind != SOURCE_JSON_STRING || !value->text.length
        || memchr(value->text.bytes, 0, value->text.length)) {
        return source_json_schema_error(value, issue, "Expected a nonempty NUL-free protocol string");
    }
    *output = (const char *)value->text.bytes;
    return SOURCE_JSON_OK;
}
/** Bound each decimal step before multiplication in the full unsigned range.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) forbids numeric
 * truncation; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains the original failure site without publishing a partial number.
 */
SourceJsonStatus source_json_schema_unsigned(const SourceJsonValue *value, uintmax_t *output, SourceJsonIssue *issue) {
    if (!issue || !output || !value || value->kind != SOURCE_JSON_STRING || !value->text.length) {
        return source_json_schema_error(value, issue, "Expected a canonical unsigned decimal protocol string");
    }
    const unsigned char *bytes = value->text.bytes;
    if (bytes[0] == '0' && value->text.length != 1) {
        return source_json_schema_error(value, issue, "Noncanonical leading zero in a protocol integer");
    }
    uintmax_t number = 0;
    for (size_t index = 0; index < value->text.length; index++) {
        if (bytes[index] < '0' || bytes[index] > '9') {
            return source_json_schema_error(value, issue, "Nondigit in a protocol integer");
        }
        uintmax_t digit = bytes[index] - '0';
        if (number > (UINTMAX_MAX - digit) / 10) {
            return source_json_schema_error(value, issue, "Protocol integer exceeds its target range");
        }
        number = number * 10 + digit;
    }
    *output = number;
    return SOURCE_JSON_OK;
}
SourceJsonStatus source_json_schema_size(const SourceJsonValue *value, size_t *output, SourceJsonIssue *issue) {
    if (!output) { return source_json_schema_error(value, issue, "Missing protocol size output"); }
    uintmax_t number = 0;
    SourceJsonStatus status = source_json_schema_unsigned(value, &number, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    size_t converted = (size_t)number;
    if ((uintmax_t)converted != number) {
        return source_json_schema_error(value, issue, "Protocol integer exceeds size_t");
    }
    *output = converted;
    return SOURCE_JSON_OK;
}
