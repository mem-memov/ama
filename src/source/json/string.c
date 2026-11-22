/** @file
 * @brief Decode exact JSON strings and paired Unicode escapes with checked storage.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves
 * bytes and original token sites; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains failed string allocations in the response operation's arena.
 */
#include <stdint.h>
#include "internal.h"
/** Decode one ASCII hexadecimal nibble without locale or replacement characters.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static int hexadecimal(unsigned char byte) {
    if (byte >= '0' && byte <= '9') { return byte - '0'; }
    if (byte >= 'a' && byte <= 'f') { return byte - 'a' + 10; }
    if (byte >= 'A' && byte <= 'F') { return byte - 'A' + 10; }
    return -1;
}
/** Consume exactly four hexadecimal digits into one UTF-16 code unit.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool unit(SourceJsonCursor *cursor, uint32_t *output) {
    if (cursor->length - cursor->offset < 4) {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Incomplete JSON Unicode escape");
    }
    uint32_t value = 0;
    for (size_t index = 0; index < 4; index++) {
        int digit = hexadecimal(cursor->bytes[cursor->offset]);
        if (digit < 0) { return source_json_fail(cursor, SOURCE_JSON_INVALID, "Invalid JSON Unicode escape"); }
        cursor->offset++; value = value * 16 + (unsigned)digit;
    }
    *output = value;
    return true;
}
/** Count and optionally emit one valid scalar, including escaped NUL.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static void scalar(unsigned char *output, size_t *length, uint32_t value) {
    unsigned char bytes[4]; size_t count;
    if (value <= 0x7f) { bytes[0] = (unsigned char)value; count = 1; }
    else if (value <= 0x7ff) {
        bytes[0] = (unsigned char)(0xc0 | (value >> 6));
        bytes[1] = (unsigned char)(0x80 | (value & 0x3f)); count = 2;
    } else if (value <= 0xffff) {
        bytes[0] = (unsigned char)(0xe0 | (value >> 12));
        bytes[1] = (unsigned char)(0x80 | ((value >> 6) & 0x3f));
        bytes[2] = (unsigned char)(0x80 | (value & 0x3f)); count = 3;
    } else {
        bytes[0] = (unsigned char)(0xf0 | (value >> 18));
        bytes[1] = (unsigned char)(0x80 | ((value >> 12) & 0x3f));
        bytes[2] = (unsigned char)(0x80 | ((value >> 6) & 0x3f));
        bytes[3] = (unsigned char)(0x80 | (value & 0x3f)); count = 4;
    }
    for (size_t index = 0; index < count; index++) {
        if (output) { output[*length] = bytes[index]; }
        (*length)++;
    }
}
/** Each escape emits no more bytes than it consumes; decoded length therefore
 * cannot exceed the remaining captured input. Raw UTF-8 was validated once.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool scan(SourceJsonCursor *cursor, unsigned char *output, size_t *length) {
    if (cursor->offset == cursor->length || cursor->bytes[cursor->offset] != '"') {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Expected a quoted JSON string");
    }
    cursor->offset++;
    while (cursor->offset < cursor->length) {
        size_t position = cursor->offset;
        unsigned char byte = cursor->bytes[cursor->offset++];
        if (byte == '"') { return true; }
        if (byte < 0x20) {
            cursor->offset = position;
            return source_json_fail(cursor, SOURCE_JSON_INVALID, "Unescaped JSON string control byte");
        }
        if (byte != '\\') {
            if (output) { output[*length] = byte; }
            (*length)++; continue;
        }
        if (cursor->offset == cursor->length) {
            return source_json_fail(cursor, SOURCE_JSON_INVALID, "Incomplete JSON string escape");
        }
        byte = cursor->bytes[cursor->offset++];
        uint32_t value;
        switch (byte) {
            case '"': case '\\': case '/': value = byte; break;
            case 'b': value = '\b'; break;
            case 'f': value = '\f'; break;
            case 'n': value = '\n'; break;
            case 'r': value = '\r'; break;
            case 't': value = '\t'; break;
            case 'u':
                position = cursor->offset;
                if (!unit(cursor, &value)) { return false; }
                if (value >= 0xd800 && value <= 0xdbff) {
                    if (cursor->length - cursor->offset < 2 || cursor->bytes[cursor->offset] != '\\'
                        || cursor->bytes[cursor->offset + 1] != 'u') {
                        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Missing JSON low surrogate");
                    }
                    cursor->offset += 2;
                    size_t low_position = cursor->offset;
                    uint32_t low;
                    if (!unit(cursor, &low)) { return false; }
                    if (low < 0xdc00 || low > 0xdfff) {
                        cursor->offset = low_position;
                        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Invalid JSON low surrogate");
                    }
                    value = 0x10000 + (value - 0xd800) * 0x400 + low - 0xdc00;
                } else if (value >= 0xdc00 && value <= 0xdfff) {
                    cursor->offset = position;
                    return source_json_fail(cursor, SOURCE_JSON_INVALID, "Unpaired JSON low surrogate");
                }
                break;
            default:
                cursor->offset--;
                return source_json_fail(cursor, SOURCE_JSON_INVALID, "Unknown JSON string escape");
        }
        scalar(output, length, value);
    }
    return source_json_fail(cursor, SOURCE_JSON_INVALID, "Unclosed JSON string");
}
bool source_json_string(SourceJsonCursor *cursor, SourceJsonText *output) {
    size_t begin = cursor->offset, length = 0;
    if (!scan(cursor, NULL, &length)) { return false; }
    if (length == SIZE_MAX) { return source_json_fail(cursor, SOURCE_JSON_OUT_OF_MEMORY, "JSON string is too large"); }
    unsigned char *bytes = source_arena_allocate(cursor->arena, length + 1);
    if (!bytes) { return source_json_fail(cursor, SOURCE_JSON_OUT_OF_MEMORY, "Cannot allocate the decoded JSON string"); }
    SourceJsonCursor replay = *cursor;
    replay.offset = begin; size_t written = 0;
    if (!scan(&replay, bytes, &written) || written != length || replay.offset != cursor->offset) {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "JSON response changed during string decoding");
    }
    bytes[length] = 0;
    *output = (SourceJsonText){bytes, length, {cursor->source, begin, cursor->offset}};
    return true;
}
