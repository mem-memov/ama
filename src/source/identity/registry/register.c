#include "../internal.h"
#include "../hash/sha256.h"

void identity_registry_init(IdentityRegistry *registry, SourceAllocator allocator) {
    *registry = (IdentityRegistry){0};
    source_arena_init(&registry->arena, allocator);
}

void identity_registry_destroy(IdentityRegistry *registry) {
    source_arena_destroy(&registry->arena);
    registry->first = NULL;
    registry->conflicting = NULL;
}

IdentityStatus identity_register(IdentityRegistry *registry,
    QualifiedIdentity identity, const IdentityName **output) {
    registry->conflicting = NULL;
    IdentityEncoding encoding;
    IdentityStatus status = identity_encode(&registry->arena, identity, &encoding);
    if (status != IDENTITY_OK) {
        return status;
    }
    unsigned char digest[32];
    identity_sha256(encoding.bytes, encoding.length, digest);
    return identity_insert(registry, encoding, digest, output);
}
