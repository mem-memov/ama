/**
 * @file
 * @brief Copy a measured original identity display once without retaining repeated member-prefix copies.
 *
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01) preserves
 * original spelling. Display is explanatory text; counted binary key fields,
 * not this delimiter-joined string, determine identity under
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02).
 */
#include <string.h>
#include "internal.h"

/** Allocate one exact terminated string, then copy each immutable source part once.
 * Measurement proves total capacity and terminator bounds; no fallible operation
 * remains after allocation, and no normalization or repeated concatenation occurs.
 * Component contract: [NAME-01](../../../../spec/implementation/compiler.md#name-01), [NAME-02](../../../../spec/implementation/compiler.md#name-02). */
const char *identity_encoding_display(SourceArena *arena, QualifiedIdentity identity, const IdentityEncodingShape *shape) {
    char *display = source_arena_allocate(arena, shape->display_length + 1);
    if (display == NULL) { return NULL; }
    size_t length = strlen(identity.module_path);
    memcpy(display, identity.module_path, length);
    char *cursor = display + length;
    for (size_t index = 0; index < identity.member_count; index++) {
        *cursor++ = ':';
        *cursor++ = ':';
        length = strlen(identity.members[index]);
        memcpy(cursor, identity.members[index], length);
        cursor += length;
    }
    *cursor = 0;
    return display;
}
