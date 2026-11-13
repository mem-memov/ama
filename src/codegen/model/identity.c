/**
 * @file
 * @brief Map exact resolved type/member objects and register qualified shared output names.
 *
 * [NAME-01](../../../spec/implementation/compiler.md#name-01)
 * Runtime type ordinals are assembly-local registry keys, not cross-build external identities. Source names use complete registered keys.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Exact semantic base names do not require a synthetic declaration AST.
 */
#include "codegen/internal.h"

/**
 * Return the exact descriptor pointer ordinal starting at one, or zero if absent. This is an
 * assembly-local registry key, not a stable external type identity.
 */
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
 */
const ScaffoldMember *codegen_member(const ScaffoldType *type, const SemanticSymbol *symbol) {
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        if (member->field->symbol == symbol) { return member; }
    }
    return NULL;
}

/**
 * Register a full module/category/member identity using exact source bytes and return
 * scaffold-owned generated C spelling. Preserve collision/allocation failure; no
 * transliteration.
 */
const char *codegen_constant_name(NativeScaffold *scaffold, const SemanticConstantPlan *constant) {
    SourceSpan name = constant->symbol->name;
    size_t length = name.end - name.start;
    char *member = scaffold_allocate(scaffold, length + 1);
    if (member == NULL) { return NULL; }
    for (size_t index = 0; index < length; index++) { member[index] = (char)source_bytes(name.source)[name.start + index]; }
    const char *component = member;
    QualifiedIdentity key = {IDENTITY_CONSTANT, constant->symbol->owner->module->source->logical_path,
        &component, 1, name};
    const IdentityName *identity = NULL;
    IdentityStatus status = identity_register(&scaffold->identities, key, &identity);
    if (status != IDENTITY_OK) {
        scaffold_error(scaffold, status == IDENTITY_COLLISION ? DIAGNOSTIC_IDENTITY_COLLISION : DIAGNOSTIC_OUT_OF_MEMORY,
            "Cannot register a qualified shared value identity", NULL);
        return NULL;
    }
    return identity->internal_name;
}
