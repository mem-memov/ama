/** @file
 * @brief Advance the exact JSON object/array grammar without recursive C calls.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) gives each
 * pending container an arena-owned continuation; [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * retains original byte spans instead of guessing missing punctuation.
 */
#include "internal.h"
/** Complete one container; its parent already awaits a separator from attachment.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool close_frame(SourceJsonCursor *cursor) {
    cursor->offset++;
    cursor->frame->value->origin.end = cursor->offset;
    cursor->frame = cursor->frame->parent;
    return true;
}
bool source_json_container(SourceJsonCursor *cursor) {
    source_json_space(cursor);
    SourceJsonFrame *frame = cursor->frame;
    if (cursor->offset == cursor->length) {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Unclosed JSON container");
    }
    unsigned char byte = cursor->bytes[cursor->offset];
    switch (frame->state) {
        case JSON_FIRST_KEY:
            if (byte == '}') { return close_frame(cursor); }
            /* The first key otherwise has the same quoted-token requirement. */
            /* fall through */
        case JSON_KEY:
            if (!source_json_string(cursor, &frame->key)) { return false; }
            frame->state = JSON_COLON;
            return true;
        case JSON_COLON:
            if (byte != ':') { return source_json_fail(cursor, SOURCE_JSON_INVALID, "Missing JSON object colon"); }
            cursor->offset++; frame->state = JSON_VALUE;
            return true;
        case JSON_FIRST_VALUE:
            if (byte == ']') { return close_frame(cursor); }
            /* fall through */
        case JSON_VALUE:
            return source_json_value(cursor);
        case JSON_SEPARATOR:
            if (byte == (frame->value->kind == SOURCE_JSON_OBJECT ? '}' : ']')) { return close_frame(cursor); }
            if (byte != ',') { return source_json_fail(cursor, SOURCE_JSON_INVALID, "Missing JSON container separator"); }
            cursor->offset++;
            frame->state = frame->value->kind == SOURCE_JSON_OBJECT ? JSON_KEY : JSON_VALUE;
            return true;
    }
    return source_json_fail(cursor, SOURCE_JSON_INVALID, "Invalid JSON container state");
}
