/**
 * @file
 * @brief Map exact resolved type/member objects and register qualified shared output names.
 *
 * [NAME-01](../../../spec/implementation/compiler.md#name-01)
 * Runtime type ordinals are assembly-local registry keys, not cross-build external identities. Source names use complete registered keys.
 * [CONST-06](../../../spec/language/declarations.md#const-06) keeps captured owners distinct in shared names.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Exact semantic base names do not require a synthetic declaration AST.
 */
#include "codegen/internal.h"
#include "semantic/specializations/members/key.h"

/**
 * Return the exact descriptor pointer ordinal starting at one, or zero if absent. This is an
 * assembly-local registry key, not a stable external type identity.
 * Component contract: [NAME-01](../../../spec/implementation/compiler.md#name-01), [TYPE-06](../../../spec/language/declarations.md#type-06). */
size_t codegen_type_id(const NativeScaffold *scaffold, const ScaffoldType *type) {
    size_t index = 1;
    for (const ScaffoldType *candidate = scaffold->types; candidate != NULL; candidate = candidate->next, index++) {
        if (candidate == type) { return index; }
    }
    return 0;
}

/**
 * Find the exact resolved symbol pointer in a known type, or null. Caller supplies a registered
 * owner; no name matching or allocation.
 * Component contract: [NAME-01](../../../spec/implementation/compiler.md#name-01), [TYPE-06](../../../spec/language/declarations.md#type-06). */
const ScaffoldMember *codegen_member(const ScaffoldType *type, const SemanticSymbol *symbol) {
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        if (member->field->symbol == symbol) { return member; }
    }
    return NULL;
}

/**
 * Register the complete owner substitution plus original member and return
 * scaffold-owned generated C spelling. Registration copies the framed identity
 * before temporary descriptors expire. Preserve the first collision/allocation
 * failure and publish no runtime storage here.
 * [CONST-06](../../../spec/language/declarations.md#const-06) separates captured namespaces.
 * [NAME-02](../../../spec/implementation/compiler.md#name-02) compares complete keys before sharing names.
 */
const char *codegen_constant_name(NativeScaffold *scaffold, const SemanticConstantPlan *constant) {
    if (scaffold->error.message) { return NULL; }
    SourceArena temporary;
    source_arena_init(&temporary, scaffold->arena.allocator);
    QualifiedIdentity key;
    IdentityStatus status = semantic_constant_key(&scaffold->frontend->semantic, constant->symbol, &temporary, &key);
    const IdentityName *identity = NULL;
    if (status == IDENTITY_OK) { status = identity_register(&scaffold->identities, key, &identity); }
    source_arena_destroy(&temporary);
    if (status != IDENTITY_OK) {
        scaffold_error(scaffold, status == IDENTITY_COLLISION ? DIAGNOSTIC_IDENTITY_COLLISION
                : status == IDENTITY_OUT_OF_MEMORY || status == IDENTITY_LIMIT ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_TYPE_MISMATCH,
            "Cannot register a qualified shared value identity", NULL);
        return NULL;
    }
    return identity->internal_name;
}
