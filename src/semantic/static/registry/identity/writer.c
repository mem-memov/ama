/** @file
 * @brief Measure/write exact counted UTF-8 key fields with checked arithmetic.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) preserves original UTF-8 bytes.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) forbids truncation or delimiter guessing.
 */
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "internal.h"

/** Measurement and output share identical arithmetic; no truncated prefix is
 * accepted after a limit failure. Null data denotes measurement only.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02).
 */
void static_key_bytes(StaticKeyWriter *writer, const char *bytes, size_t length) {
    if (writer->status != STATIC_KEY_OK) { return; }
    if (length > SIZE_MAX - writer->length ||
        (writer->data && (writer->length > writer->capacity || length > writer->capacity - writer->length))) {
        writer->status = STATIC_KEY_LIMIT;
        return;
    }
    if (writer->data && length) { memcpy(writer->data + writer->length, bytes, length); }
    writer->length += length;
}

/** One decimal digit per size_t bit is a conservative count buffer bound.
 * Division strictly decreases every positive value; zero emits one digit.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) fixes decimal framing.
 */
void static_key_number(StaticKeyWriter *writer, size_t value) {
    char digits[sizeof(size_t) * CHAR_BIT + 1];
    size_t count = 0;
    do {
        digits[count++] = (char)('0' + value % 10);
        value /= 10;
    } while (value);
    while (count) { static_key_bytes(writer, &digits[--count], 1); }
    static_key_bytes(writer, ":", 1);
}

/** UTF-8 is retained byte-for-byte; byte counts distinguish every boundary even
 * when a name contains spaces, dots or framing punctuation.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) preserves full identities.
 */
void static_key_text(StaticKeyWriter *writer, const char *bytes, size_t length) {
    static_key_number(writer, length);
    static_key_bytes(writer, bytes, length);
}
