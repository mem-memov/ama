/**
 * @file
 * @brief Own complete encoded identities and collision-checked internal names.
 *
 * [NAME-02](../../../spec/implementation/compiler.md#name-02)
 * Hash equality never merges distinct full keys; source declaration spans remain borrowed.
 */
#ifndef AMA_SOURCE_IDENTITY_H
#define AMA_SOURCE_IDENTITY_H

#include "source/memory/arena.h"
#include "source/text/source.h"

/** Disjoint internal naming categories, including the native namespace category. */
typedef enum {
    IDENTITY_TYPE = 1,
    IDENTITY_CONSTANT,
    IDENTITY_STATE_MEMBER,
    IDENTITY_INTERFACE_MEMBER,
    IDENTITY_LABEL,
    IDENTITY_RUNTIME,
    IDENTITY_NATIVE_NAMESPACE
} IdentityCategory;

/** Borrowed complete supported identity and its original declaration span. */
typedef struct {
    IdentityCategory category;
    const char *module_path;
    const char *const *members;
    size_t member_count;
    SourceSpan declaration;
} QualifiedIdentity;

/** Registry-owned complete key/digest/internal name; source spans borrow their manager. */
typedef struct IdentityName {
    struct IdentityName *next;
    IdentityCategory category;
    unsigned char digest[32];
    const unsigned char *encoding;
    size_t encoding_length;
    const char *source_name;
    SourceSpan declaration;
    char internal_name[71];
} IdentityName;

/** Exact registration outcomes; collision is a diagnosed full-key disagreement. */
typedef enum {
    IDENTITY_OK,
    IDENTITY_INVALID,
    IDENTITY_OUT_OF_MEMORY,
    IDENTITY_LIMIT,
    IDENTITY_COLLISION
} IdentityStatus;

/** Owner of names/encodings and a borrowed pointer to the last conflicting registry record. */
typedef struct {
    SourceArena arena;
    IdentityName *first;
    const IdentityName *conflicting;
} IdentityRegistry;

/* Original identities and generated names are owned until destroy. Source spans
 * borrow their manager. Repeat registration returns the same record. Failure
 * never publishes a name or merges distinct identities. */
/** Start an empty registry with explicit allocator authority; declarations borrow their source managers.
 */
void identity_registry_init(IdentityRegistry *registry, SourceAllocator allocator);
/** Release all encodings/names; every returned IdentityName pointer expires.
 */
void identity_registry_destroy(IdentityRegistry *registry);
/** Encode a supported complete identity, hash it, and compare full keys before publication.
 * @param[out] output Changes only on IDENTITY_OK; repeats reuse the same record.
 * @return Malformed identity, checked limit, allocation failure, unequal-key collision or success. Failed scratch remains arena-owned until destroy.
 */
IdentityStatus identity_register(IdentityRegistry *registry,
    QualifiedIdentity identity, const IdentityName **output);

#endif
