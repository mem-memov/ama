/** @file
 * @brief Register complete captured view/member/prefix keys before temporary descriptor teardown.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) makes substitutions part of nominal identity.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) requires independently framed role components.
 */
#include <stdint.h>
#include "identity.h"
#include "native/scaffold/internal.h"
#include "semantic/specializations/identity/key.h"

/** Append one borrowed role using checked component counts and a fresh pointer array.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) prevents concatenation ambiguity.
 */
static IdentityStatus append(SourceArena *arena, QualifiedIdentity *key, const char *member) {
    if (!member) { return IDENTITY_OK; }
    if (key->member_count == SIZE_MAX || key->member_count + 1 > SIZE_MAX / sizeof(const char *)) { return IDENTITY_LIMIT; }
    const char **parts = source_arena_allocate(arena, (key->member_count + 1) * sizeof(*parts));
    if (!parts) { return IDENTITY_OUT_OF_MEMORY; }
    for (size_t index = 0; index < key->member_count; index++) { parts[index] = key->members[index]; }
    parts[key->member_count] = member;
    key->members = parts;
    key->member_count++;
    return IDENTITY_OK;
}

const IdentityName *scaffold_identity(NativeScaffold *scaffold, const SemanticType *type,
    IdentityCategory category, const char *member, SourceSpan site) {
    if (scaffold->error.message) { return NULL; }
    SourceArena temporary;
    source_arena_init(&temporary, scaffold->arena.allocator);
    QualifiedIdentity key;
    IdentityStatus status = semantic_type_key(&scaffold->frontend->semantic, type, &temporary, &key);
    if (status == IDENTITY_OK) { status = append(&temporary, &key, member); }
    const IdentityName *name = NULL;
    if (status == IDENTITY_OK) {
        key.category = category;
        key.declaration = site;
        status = identity_register(&scaffold->identities, key, &name);
    }
    source_arena_destroy(&temporary);
    if (status != IDENTITY_OK) {
        scaffold_error(scaffold, status == IDENTITY_COLLISION ? DIAGNOSTIC_IDENTITY_COLLISION
                : status == IDENTITY_OUT_OF_MEMORY || status == IDENTITY_LIMIT ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_TYPE_MISMATCH,
            "A complete generated native identity could not be registered", NULL);
    }
    return name;
}
