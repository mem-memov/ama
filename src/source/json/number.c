/** @file
 * @brief Preserve exact JSON decimal tokens without numeric conversion.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * shares immutable protocol syntax without sharing project mutation or compiler owners;
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires
 * exact bytes and source coordinates rather than lossy scalar conversions;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains
 * partial trees and decoded strings under one explicit operation arena.
 */
#include "internal.h"
/** ASCII decimal digits have no locale-specific numeric meaning.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static bool digit(unsigned char byte) { return byte >= '0' && byte <= '9'; }
bool source_json_number(SourceJsonCursor *cursor, SourceJsonText *output) {
    size_t begin = cursor->offset;
    if (cursor->bytes[cursor->offset] == '-') { cursor->offset++; }
    if (cursor->offset == cursor->length || !digit(cursor->bytes[cursor->offset])) {
        return source_json_fail(cursor, SOURCE_JSON_INVALID, "Expected a JSON integer digit");
    }
    if (cursor->bytes[cursor->offset] == '0') {
        cursor->offset++;
        if (cursor->offset < cursor->length && digit(cursor->bytes[cursor->offset])) {
            return source_json_fail(cursor, SOURCE_JSON_INVALID, "Leading zero in a JSON number");
        }
    } else {
        do { cursor->offset++; } while (cursor->offset < cursor->length && digit(cursor->bytes[cursor->offset]));
    }
    if (cursor->offset < cursor->length && cursor->bytes[cursor->offset] == '.') {
        cursor->offset++;
        size_t first = cursor->offset;
        while (cursor->offset < cursor->length && digit(cursor->bytes[cursor->offset])) { cursor->offset++; }
        if (cursor->offset == first) { return source_json_fail(cursor, SOURCE_JSON_INVALID, "Empty JSON number fraction"); }
    }
    if (cursor->offset < cursor->length && (cursor->bytes[cursor->offset] == 'e' || cursor->bytes[cursor->offset] == 'E')) {
        cursor->offset++;
        if (cursor->offset < cursor->length && (cursor->bytes[cursor->offset] == '+' || cursor->bytes[cursor->offset] == '-')) { cursor->offset++; }
        size_t first = cursor->offset;
        while (cursor->offset < cursor->length && digit(cursor->bytes[cursor->offset])) { cursor->offset++; }
        if (cursor->offset == first) { return source_json_fail(cursor, SOURCE_JSON_INVALID, "Empty JSON number exponent"); }
    }
    *output = (SourceJsonText){cursor->bytes + begin, cursor->offset - begin,
        {cursor->source, begin, cursor->offset}};
    return true;
}
