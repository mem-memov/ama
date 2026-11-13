/**
 * @file
 * @brief Publish one completely measured category/root-aware identity encoding and original display spelling.
 *
 * [NAME-01](../../../../spec/implementation/compiler.md#name-01) uses explicit
 * version-two root tags and original counted components. [NAME-02](../../../../spec/implementation/compiler.md#name-02)
 * requires complete immutable key bytes before hashing or registry publication.
 */
#include <string.h>
#include "internal.h"

/** Allocate private encoding/display ownership, then fill the exact versioned key.
 * Any failed allocation leaves arena-owned scratch but changes no output view.
 */
IdentityStatus identity_encode(SourceArena *arena, QualifiedIdentity identity, IdentityEncoding *output) {
    IdentityEncodingShape shape;
    IdentityStatus status = identity_encoding_measure(identity, &shape);
    if (status != IDENTITY_OK) { return status; }
    unsigned char *bytes = source_arena_allocate(arena, shape.length);
    if (bytes == NULL) { return IDENTITY_OUT_OF_MEMORY; }
    const char *display = identity_encoding_display(arena, identity, &shape);
    if (display == NULL) { return IDENTITY_OUT_OF_MEMORY; }
    const unsigned char prefix[] = {'A','M','A','-','I','D',0,2};
    memcpy(bytes, prefix, sizeof(prefix));
    bytes[8] = (unsigned char)identity.category;
    bytes[9] = (unsigned char)shape.root_kind;
    identity_encoding_parts(bytes + 10, identity, &shape);
    *output = (IdentityEncoding){bytes, shape.length, display, identity.category, identity.declaration};
    return IDENTITY_OK;
}
