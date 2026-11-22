/** @file
 * @brief Validate exact object field sets before publishing borrowed values.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * shares exact data validation without sharing compiler or mutation owners;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves
 * lossless bytes and exact integer offsets without permissive conversion;
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains
 * complete/failed decoded payloads under the caller's operation arena.
 */
#include <string.h>
#include "internal.h"
/** Decoded lengths prevent escaped embedded NUL from truncating a field key.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool key(const SourceJsonValue *value, const char *name) {
    size_t length = strlen(name);
    return value->key.length == length && !memcmp(value->key.bytes, name, length);
}
SourceJsonStatus source_json_schema_fields(const SourceJsonValue *value, const char *const *names,
    size_t count, const SourceJsonValue **output, SourceJsonIssue *issue) {
    if (!issue || !value || value->kind != SOURCE_JSON_OBJECT || (count && (!names || !output))) {
        return source_json_schema_error(value, issue, "Expected a complete protocol object schema");
    }
    size_t observed = 0;
    for (const SourceJsonValue *child = value->first; child; child = child->next) {
        bool known = false;
        for (size_t index = 0; index < count; index++) {
            if (key(child, names[index])) { known = true; break; }
        }
        if (!known) { return source_json_schema_error(child, issue, "Unknown protocol object field"); }
        observed++;
    }
    if (observed != count) { return source_json_schema_error(value, issue, "Missing protocol object field"); }
    /* The syntax parser proved decoded-key uniqueness. Every requested field
     * therefore exists exactly once before this second publication pass. */
    for (size_t index = 0; index < count; index++) {
        for (const SourceJsonValue *child = value->first; child; child = child->next) {
            if (key(child, names[index])) { output[index] = child; break; }
        }
    }
    return SOURCE_JSON_OK;
}
