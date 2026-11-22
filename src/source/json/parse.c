/** @file
 * @brief Publish one complete strict JSON protocol tree after finite iterative parsing.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves
 * response bytes and rejects malformed UTF-8; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * keeps failed scratch owned and leaves caller output unchanged.
 */
#include "internal.h"
#include "source/unicode/unicode.h"
SourceJsonStatus source_json_parse(SourceArena *arena, const SourceText *source,
    const SourceJsonValue **output, SourceJsonIssue *issue) {
    if (!arena || !source || !output || !issue) { return SOURCE_JSON_INVALID; }
    SourceJsonCursor cursor = {.arena = arena, .source = source,
        .bytes = source_bytes(source), .length = source_length(source)};
    size_t invalid;
    if (!utf8_validate(cursor.bytes, cursor.length, &invalid)) {
        cursor.offset = invalid;
        (void)source_json_fail(&cursor, SOURCE_JSON_INVALID, "JSON response is not valid UTF-8");
    } else if (source_json_value(&cursor)) {
        while (cursor.frame && source_json_container(&cursor)) {}
        if (cursor.status == SOURCE_JSON_OK) {
            source_json_space(&cursor);
            if (cursor.offset != cursor.length) {
                (void)source_json_fail(&cursor, SOURCE_JSON_INVALID, "Unexpected bytes after the JSON document");
            }
        }
    }
    if (cursor.status != SOURCE_JSON_OK) { *issue = cursor.issue; return cursor.status; }
    *output = cursor.root;
    return SOURCE_JSON_OK;
}
