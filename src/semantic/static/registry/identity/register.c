/** @file
 * @brief Register complete static keys through existing SHA-256 collision checks with temporary codec ownership.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) reuses deterministic category/root naming.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) retains full encoded identities.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) separates temporary and persistent failure storage.
 */
#include "key.h"

/** Translate the naming service's complete status domain without changing its
 * collision witness or inventing suffixes/fallback identities.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02).
 */
static StaticKeyStatus naming_status(IdentityStatus status) {
    switch (status) {
        case IDENTITY_OK: return STATIC_KEY_OK;
        case IDENTITY_INVALID: return STATIC_KEY_INVALID;
        case IDENTITY_OUT_OF_MEMORY: return STATIC_KEY_OUT_OF_MEMORY;
        case IDENTITY_LIMIT: return STATIC_KEY_LIMIT;
        case IDENTITY_COLLISION: return STATIC_KEY_COLLISION;
    }
    return STATIC_KEY_INVALID;
}

/** The naming service copies every discriminator before temporary destruction.
 * On refusal, partial persistent encodings remain in its own arena; no public
 * name receiver is changed. The term store is not a destination arena.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) preserves owner cleanup boundaries.
 */
StaticKeyStatus static_key_register(const StaticTermStore *terms, const StaticTermEnvironment *environment,
    IdentityRegistry *names, const IdentityName **output) {
    if (names == NULL) { return STATIC_KEY_INVALID; }
    names->conflicting = NULL;
    if (terms == NULL || output == NULL) { return STATIC_KEY_INVALID; }
    SourceArena temporary;
    source_arena_init(&temporary, names->arena.allocator);
    StaticKeyEncoding key;
    StaticKeyStatus status = static_key_encode(terms, environment, &temporary, &key);
    if (status == STATIC_KEY_OK) { status = naming_status(identity_register(names, key.qualified, output)); }
    source_arena_destroy(&temporary);
    return status;
}
