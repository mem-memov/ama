/** @file
 * @brief Decode exact redundant UTF-8/hex protocol byte values without replacement.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires
 * both representations to describe the same bytes; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains allocation failures without publishing a partial payload.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"
/** Wire hex is canonical lowercase, without prefixes, separators or guessing.
 * Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves exact protocol values; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains complete/failed operation storage.
 */
static int hexadecimal(unsigned char byte) {
    if (byte >= '0' && byte <= '9') { return byte - '0'; }
    if (byte >= 'a' && byte <= 'f') { return byte - 'a' + 10; }
    return -1;
}
SourceJsonStatus source_json_schema_bytes(SourceArena *arena, const SourceJsonValue *value,
    SourceJsonText *output, SourceJsonIssue *issue) {
    if (!arena || !output || !issue) { return source_json_schema_error(value, issue, "Invalid protocol byte decoder owner"); }
    const char *const names[] = {"utf8", "hex"};
    const SourceJsonValue *fields[2];
    SourceJsonStatus status = source_json_schema_fields(value, names, 2, fields, issue);
    if (status != SOURCE_JSON_OK) { return status; }
    const SourceJsonValue *text = fields[0], *hex = fields[1];
    if (hex->kind != SOURCE_JSON_STRING || (hex->text.length & 1)) {
        return source_json_schema_error(hex, issue, "Expected even-length protocol hex text");
    }
    size_t length = hex->text.length / 2;
    unsigned char *bytes = source_arena_allocate(arena, length + 1);
    if (!bytes) {
        *issue = (SourceJsonIssue){value->origin, "Cannot allocate decoded protocol bytes"};
        return SOURCE_JSON_OUT_OF_MEMORY;
    }
    for (size_t index = 0; index < length; index++) {
        int high = hexadecimal(hex->text.bytes[2 * index]);
        int low = hexadecimal(hex->text.bytes[2 * index + 1]);
        if (high < 0 || low < 0) { return source_json_schema_error(hex, issue, "Invalid lowercase protocol hex digit"); }
        bytes[index] = (unsigned char)((unsigned)high * 16 + (unsigned)low);
    }
    bytes[length] = 0;
    size_t invalid;
    if (utf8_validate(bytes, length, &invalid)) {
        if (text->kind != SOURCE_JSON_STRING || text->text.length != length
            || memcmp(text->text.bytes, bytes, length)) {
            return source_json_schema_error(text, issue, "Protocol UTF-8 and hex values disagree");
        }
    } else if (text->kind != SOURCE_JSON_NULL) {
        return source_json_schema_error(text, issue, "Malformed UTF-8 protocol bytes require a null text view");
    }
    *output = (SourceJsonText){bytes, length, hex->origin};
    return SOURCE_JSON_OK;
}
