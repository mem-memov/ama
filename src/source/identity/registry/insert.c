#include <string.h>
#include "../internal.h"

static void format_name(char output[71], IdentityCategory category,
    const unsigned char digest[32]) {
    static const char digits[] = "0123456789abcdef";
    static const char category_codes[] = {'t', 'c', 's', 'i', 'l', 'r'};
    memcpy(output, "ama_", 4);
    output[4] = category_codes[category - 1];
    output[5] = '_';
    for (size_t index = 0; index < 32; index++) {
        output[6 + index * 2] = digits[digest[index] >> 4];
        output[7 + index * 2] = digits[digest[index] & 15];
    }
    output[70] = 0;
}

IdentityStatus identity_insert(IdentityRegistry *registry, IdentityEncoding encoding,
    const unsigned char digest[32], const IdentityName **output) {
    registry->conflicting = NULL;
    for (IdentityName *other = registry->first; other != NULL; other = other->next) {
        if (memcmp(other->digest, digest, 32) != 0) {
            continue;
        }
        if (other->encoding_length != encoding.length
            || memcmp(other->encoding, encoding.bytes, encoding.length) != 0) {
            registry->conflicting = other;
            return IDENTITY_COLLISION;
        }
        *output = other;
        return IDENTITY_OK;
    }
    IdentityName *name = source_arena_allocate(&registry->arena, sizeof(*name));
    if (name == NULL) {
        return IDENTITY_OUT_OF_MEMORY;
    }
    *name = (IdentityName){0};
    memcpy(name->digest, digest, 32);
    name->encoding = encoding.bytes;
    name->encoding_length = encoding.length;
    name->source_name = encoding.source_name;
    name->declaration = encoding.declaration;
    format_name(name->internal_name, encoding.category, digest);
    name->next = registry->first;
    registry->first = name;
    *output = name;
    return IDENTITY_OK;
}
