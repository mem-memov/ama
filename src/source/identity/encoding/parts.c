/**
 * @file
 * @brief Serialize full original module/member components with exact big-endian lengths.
 *
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01) preserves
 * category/root/component boundaries, including original single spaces and UTF-8
 * spellings. Premeasurement supplies complete writable output capacity.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Write a complete unsigned 64-bit count without host endianness or alignment assumptions. */
static void append_number(unsigned char **cursor, uint64_t number) {
    for (size_t index = 0; index < 8; index++) { *(*cursor)++ = (unsigned char)(number >> ((7 - index) * 8)); }
}

/** Emit one measured byte string; embedded punctuation/member role text cannot merge adjacent components. */
static void append_part(unsigned char **cursor, const char *bytes, size_t length) {
    append_number(cursor, length);
    memcpy(*cursor, bytes, length);
    *cursor += length;
}

/** Skip exactly the source root marker, never serializing it as an empty component. */
void identity_encoding_parts(unsigned char *cursor, QualifiedIdentity identity, const IdentityEncodingShape *shape) {
    append_number(&cursor, shape->module_count);
    size_t start = shape->root_width;
    for (size_t index = 0; index < shape->module_count; index++) {
        size_t end = start;
        while (identity.module_path[end] != 0 && identity.module_path[end] != '.') { end++; }
        append_part(&cursor, identity.module_path + start, end - start);
        start = end + (identity.module_path[end] != 0);
    }
    append_number(&cursor, identity.member_count);
    for (size_t index = 0; index < identity.member_count; index++) {
        append_part(&cursor, identity.members[index], strlen(identity.members[index]));
    }
}
