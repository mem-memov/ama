#ifndef AMA_SOURCE_IDENTITY_H
#define AMA_SOURCE_IDENTITY_H

#include "source/memory/arena.h"
#include "source/text/source.h"

typedef enum {
    IDENTITY_TYPE = 1,
    IDENTITY_CONSTANT,
    IDENTITY_STATE_MEMBER,
    IDENTITY_INTERFACE_MEMBER,
    IDENTITY_LABEL,
    IDENTITY_RUNTIME
} IdentityCategory;

typedef struct {
    IdentityCategory category;
    const char *module_path;
    const char *const *members;
    size_t member_count;
    SourceSpan declaration;
} QualifiedIdentity;

typedef struct IdentityName {
    struct IdentityName *next;
    unsigned char digest[32];
    const unsigned char *encoding;
    size_t encoding_length;
    const char *source_name;
    SourceSpan declaration;
    char internal_name[71];
} IdentityName;

typedef enum {
    IDENTITY_OK,
    IDENTITY_INVALID,
    IDENTITY_OUT_OF_MEMORY,
    IDENTITY_LIMIT,
    IDENTITY_COLLISION
} IdentityStatus;

typedef struct {
    SourceArena arena;
    IdentityName *first;
    const IdentityName *conflicting;
} IdentityRegistry;

/* Original identities and generated names are owned until destroy. Source spans
 * borrow their manager. Repeat registration returns the same record. Failure
 * never publishes a name or merges distinct identities. */
void identity_registry_init(IdentityRegistry *registry, SourceAllocator allocator);
void identity_registry_destroy(IdentityRegistry *registry);
IdentityStatus identity_register(IdentityRegistry *registry,
    QualifiedIdentity identity, const IdentityName **output);

#endif
