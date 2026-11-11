#include <stdint.h>
#include <string.h>
#include "../internal.h"
#include "source/module/catalog.h"
#include "source/unicode/unicode.h"

_Static_assert(SIZE_MAX <= UINT64_MAX, "Identity lengths require at most 64 bits");

static void append_number(unsigned char **cursor, uint64_t number) {
    for (size_t index = 0; index < 8; index++) {
        *(*cursor)++ = (unsigned char)(number >> ((7 - index) * 8));
    }
}

static void append_part(unsigned char **cursor, const char *bytes, size_t length) {
    append_number(cursor, length);
    memcpy(*cursor, bytes, length);
    *cursor += length;
}

static IdentityStatus measure(QualifiedIdentity identity, size_t *total, size_t *module_count) {
    if (identity.category < IDENTITY_TYPE || identity.category > IDENTITY_RUNTIME
        || !module_path_valid(identity.module_path)
        || (identity.member_count != 0 && identity.members == NULL)) {
        return IDENTITY_INVALID;
    }
    *module_count = 1;
    for (const char *cursor = identity.module_path + 1; *cursor != 0; cursor++) {
        *module_count += *cursor == '.';
    }
    size_t path_length = strlen(identity.module_path);
    if (path_length > SIZE_MAX - 25
        || *module_count > (SIZE_MAX - 25 - path_length) / 8) {
        return IDENTITY_LIMIT;
    }
    *total = 25 + *module_count * 8 + path_length - *module_count;
    for (size_t index = 0; index < identity.member_count; index++) {
        if (identity.members[index] == NULL || *identity.members[index] == 0) {
            return IDENTITY_INVALID;
        }
        size_t length = strlen(identity.members[index]);
        size_t error;
        if (!utf8_validate((const unsigned char *)identity.members[index], length, &error)) {
            return IDENTITY_INVALID;
        }
        if (*total > SIZE_MAX - 8 || length > SIZE_MAX - *total - 8) {
            return IDENTITY_LIMIT;
        }
        *total += 8 + length;
    }
    return (uint64_t)*total > UINT64_MAX / 8 ? IDENTITY_LIMIT : IDENTITY_OK;
}

static void encode_parts(unsigned char *cursor, QualifiedIdentity identity, size_t module_count) {
    append_number(&cursor, module_count);
    const char *part = identity.module_path + 1;
    for (size_t index = 0; index < module_count; index++) {
        const char *end = strchr(part, '.');
        size_t length = end == NULL ? strlen(part) : (size_t)(end - part);
        append_part(&cursor, part, length);
        part += length + (end != NULL);
    }
    append_number(&cursor, identity.member_count);
    for (size_t index = 0; index < identity.member_count; index++) {
        append_part(&cursor, identity.members[index], strlen(identity.members[index]));
    }
}

IdentityStatus identity_encode(SourceArena *arena, QualifiedIdentity identity,
    IdentityEncoding *output) {
    size_t total;
    size_t module_count;
    IdentityStatus status = measure(identity, &total, &module_count);
    if (status != IDENTITY_OK) {
        return status;
    }
    unsigned char *bytes = source_arena_allocate(arena, total);
    const char *display = source_arena_copy(arena, identity.module_path);
    for (size_t index = 0; display != NULL && index < identity.member_count; index++) {
        display = source_arena_join(arena, display, "::", identity.members[index]);
    }
    if (bytes == NULL || display == NULL) {
        return IDENTITY_OUT_OF_MEMORY;
    }
    const unsigned char prefix[] = {'A', 'M', 'A', '-', 'I', 'D', 0, 1};
    memcpy(bytes, prefix, sizeof(prefix));
    bytes[8] = (unsigned char)identity.category;
    encode_parts(bytes + 9, identity, module_count);
    *output = (IdentityEncoding){bytes, total, display, identity.category, identity.declaration};
    return IDENTITY_OK;
}
