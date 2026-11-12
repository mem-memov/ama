/**
 * @file
 * @brief Separate identity encoding from exact registry insertion.
 *
 * [NAME-02](../../../spec/implementation/compiler.md#name-02)
 * Expose digest injection for collision tests while equality still compares full encodings.
 */
#ifndef AMA_SOURCE_IDENTITY_INTERNAL_H
#define AMA_SOURCE_IDENTITY_INTERNAL_H

#include "identity.h"

/** Private serialized key/display view, owned by the destination registry arena. */
typedef struct {
    unsigned char *bytes;
    size_t length;
    const char *source_name;
    IdentityCategory category;
    SourceSpan declaration;
} IdentityEncoding;

/** Measure/check before encoding; output changes only after complete bytes/display-name construction. Arena owns partial scratch until destruction.
 */
IdentityStatus identity_encode(SourceArena *arena, QualifiedIdentity identity,
    IdentityEncoding *output);
/* Digest is supplied by the SHA-256 parent; equality always compares full keys.
 * This boundary also permits collision testing without assuming SHA injectivity.
 * Encoding storage must belong to registry->arena. */
/** Accept encoding bytes/display names already owned by registry.arena and a supplied digest.
 * @return Reuse an equal full key, publish one complete name, or set conflicting for an unequal-key collision. Failure leaves output unchanged.
 */
IdentityStatus identity_insert(IdentityRegistry *registry, IdentityEncoding encoding,
    const unsigned char digest[32], const IdentityName **output);

#endif
