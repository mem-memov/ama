/**
 * @file
 * @brief Prove complete version-two identity length and explicit root/component shape before allocation.
 *
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01) separates full
 * categories/root kinds/components; length overflow is a limit error, never a
 * truncated key or a shortened digest input.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

_Static_assert(SIZE_MAX <= UINT64_MAX, "Identity lengths require at most 64 bits");
_Static_assert(MODULE_ROOT_PROJECT == 1 && MODULE_ROOT_STANDARD == 2, "Identity version two fixes root tags");

/** Add exactly one length prefix plus data only after proving both additions fit. Component contract: [NAME-01](../../../../spec/implementation/compiler.md#name-01). */
bool identity_encoding_part_size(size_t *total, size_t length) {
    if (total == NULL || *total > SIZE_MAX - 8 || (uint64_t)*total > UINT64_MAX / 8 - 8) { return false; }
    size_t prefix = *total + 8;
    if (length > SIZE_MAX - prefix || (uint64_t)length > UINT64_MAX / 8 - (uint64_t)prefix) { return false; }
    *total = prefix + length;
    return true;
}

/** Validate complete spelling and measure every field; synthetic member parts are
 * nonempty UTF-8 key components, not implicitly reinterpreted as source tokens.
 * Component contract: [NAME-01](../../../../spec/implementation/compiler.md#name-01). */
IdentityStatus identity_encoding_measure(QualifiedIdentity identity, IdentityEncodingShape *output) {
    if (identity.category < IDENTITY_TYPE || identity.category > IDENTITY_NATIVE_NAMESPACE
        || (identity.category == IDENTITY_NATIVE_NAMESPACE && identity.member_count != 0)
        || !module_path_valid(identity.module_path)
        || (identity.member_count != 0 && identity.members == NULL)) { return IDENTITY_INVALID; }
    IdentityEncodingShape shape = {.length = IDENTITY_ENCODING_FIXED_BYTES,
        .root_width = identity.module_path[1] == '/' ? 2 : 1};
    shape.root_kind = shape.root_width == 2 ? MODULE_ROOT_STANDARD : MODULE_ROOT_PROJECT;
    size_t start = shape.root_width;
    for (;;) {
        size_t end = start;
        while (identity.module_path[end] != 0 && identity.module_path[end] != '/') { end++; }
        if (!identity_encoding_part_size(&shape.length, end - start)) { return IDENTITY_LIMIT; }
        /* Every successful part reserves at least eight bytes, bounding count
         * below SIZE_MAX/8 before its increment. */
        shape.module_count++;
        if (identity.module_path[end] == 0) { shape.display_length = end; break; }
        start = end + 1;
    }
    for (size_t index = 0; index < identity.member_count; index++) {
        if (identity.members[index] == NULL || *identity.members[index] == 0) { return IDENTITY_INVALID; }
        size_t length = strlen(identity.members[index]);
        size_t error;
        if (!utf8_validate((const unsigned char *)identity.members[index], length, &error)) { return IDENTITY_INVALID; }
        if (!identity_encoding_part_size(&shape.length, length)) { return IDENTITY_LIMIT; }
        if (shape.display_length > SIZE_MAX - 2 || length > SIZE_MAX - shape.display_length - 2) { return IDENTITY_LIMIT; }
        shape.display_length += 2 + length;
    }
    if (shape.display_length == SIZE_MAX) { return IDENTITY_LIMIT; }
    *output = shape;
    return IDENTITY_OK;
}
