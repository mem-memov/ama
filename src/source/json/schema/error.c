/** @file
 * @brief Report protocol schema failures at their original JSON source value.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares exact data validation without sharing compiler or mutation owners;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * lossless bytes and exact integer offsets without permissive conversion;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * complete/failed decoded payloads under the caller's operation arena.
 */
#include "internal.h"
SourceJsonStatus source_json_schema_error(const SourceJsonValue *value, SourceJsonIssue *issue, const char *message) {
    if (issue) { *issue = (SourceJsonIssue){value ? value->origin : (SourceSpan){0}, message}; }
    return SOURCE_JSON_INVALID;
}
