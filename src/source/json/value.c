/** @file
 * @brief Attach exact JSON values and own explicit container continuations.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * keeps protocol records separate from compiler owners;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains
 * incomplete private trees without publishing a successful result.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) makes decoded keys and values exact.
 */
#include <string.h>
#include "internal.h"
/** Match a complete fixed literal prefix; enclosing grammar checks its delimiter.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool literal(SourceJsonCursor *cursor, const char *text) {
    size_t length = strlen(text);
    if (length > cursor->length - cursor->offset || memcmp(cursor->bytes + cursor->offset, text, length)) {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Invalid JSON literal");
    }
    cursor->offset += length;
    return true;
}
/** Exact decoded key equality rejects escaped aliases of the same member name.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool attach(SourceJsonCursor *cursor, SourceJsonValue *value) {
    SourceJsonFrame *frame = cursor->frame;
    if (!frame) { cursor->root = value; return true; }
    if (frame->value->kind == SOURCE_JSON_OBJECT) {
        for (const SourceJsonValue *other = frame->value->first; other; other = other->next) {
            if (other->key.length == frame->key.length
                && !memcmp(other->key.bytes, frame->key.bytes, frame->key.length)) {
                cursor->offset = frame->key.origin.start;
                return source_json_fail(cursor, SOURCE_JSON_INVALID, "Duplicate decoded JSON object key");
            }
        }
        value->key = frame->key; frame->key = (SourceJsonText){0};
    }
    if (frame->last) { frame->last->next = value; }
    else { frame->value->first = value; }
    frame->last = value; frame->state = JSON_SEPARATOR;
    return true;
}
bool source_json_value(SourceJsonCursor *cursor) {
    source_json_space(cursor);
    if (cursor->offset == cursor->length) {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Missing JSON value");
    }
    size_t begin = cursor->offset;
    SourceJsonValue *value = source_arena_allocate(cursor->arena, sizeof(*value));
    if (!value) { return source_json_fail(cursor, SOURCE_JSON_OUT_OF_MEMORY, "Cannot allocate a JSON value"); }
    *value = (SourceJsonValue){0};
    unsigned char byte = cursor->bytes[cursor->offset];
    if (byte == '{' || byte == '[') {
        value->kind = byte == '{' ? SOURCE_JSON_OBJECT : SOURCE_JSON_ARRAY;
        cursor->offset++;
    } else if (byte == '"') {
        value->kind = SOURCE_JSON_STRING;
        if (!source_json_string(cursor, &value->text)) { return false; }
    } else if (byte == '-' || (byte >= '0' && byte <= '9')) {
        value->kind = SOURCE_JSON_NUMBER;
        if (!source_json_number(cursor, &value->text)) { return false; }
    } else if (byte == 't' || byte == 'f') {
        value->kind = SOURCE_JSON_BOOLEAN; value->boolean = byte == 't';
        if (!literal(cursor, value->boolean ? "true" : "false")) { return false; }
    } else if (byte == 'n') {
        value->kind = SOURCE_JSON_NULL;
        if (!literal(cursor, "null")) { return false; }
    } else { return source_json_fail(cursor, SOURCE_JSON_INVALID, "Unknown JSON value"); }
    value->origin = (SourceSpan){cursor->source, begin, cursor->offset};
    if (!attach(cursor, value)) { return false; }
    if (value->kind == SOURCE_JSON_OBJECT || value->kind == SOURCE_JSON_ARRAY) {
        SourceJsonFrame *frame = source_arena_allocate(cursor->arena, sizeof(*frame));
        if (!frame) { return source_json_fail(cursor, SOURCE_JSON_OUT_OF_MEMORY, "Cannot allocate JSON container state"); }
        *frame = (SourceJsonFrame){.parent = cursor->frame, .value = value,
            .state = value->kind == SOURCE_JSON_OBJECT ? JSON_FIRST_KEY : JSON_FIRST_VALUE};
        cursor->frame = frame;
    }
    return true;
}
