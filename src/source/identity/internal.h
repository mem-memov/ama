#ifndef AMA_SOURCE_IDENTITY_INTERNAL_H
#define AMA_SOURCE_IDENTITY_INTERNAL_H

#include "identity.h"

typedef struct {
    unsigned char *bytes;
    size_t length;
    const char *source_name;
    IdentityCategory category;
    SourceSpan declaration;
} IdentityEncoding;

IdentityStatus identity_encode(SourceArena *arena, QualifiedIdentity identity,
    IdentityEncoding *output);
/* Digest is supplied by the SHA-256 parent; equality always compares full keys.
 * This boundary also permits collision testing without assuming SHA injectivity.
 * Encoding storage must belong to registry->arena. */
IdentityStatus identity_insert(IdentityRegistry *registry, IdentityEncoding encoding,
    const unsigned char digest[32], const IdentityName **output);

#endif
