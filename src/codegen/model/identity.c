#include "codegen/internal.h"

size_t codegen_type_id(const NativeScaffold *scaffold, const ScaffoldType *type) {
    size_t index = 1;
    for (const ScaffoldType *candidate = scaffold->types; candidate != NULL; candidate = candidate->next, index++) {
        if (candidate == type) { return index; }
    }
    return 0;
}

const ScaffoldMember *codegen_member(const ScaffoldType *type, const SemanticSymbol *symbol) {
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        if (member->field->symbol == symbol) { return member; }
    }
    return NULL;
}

const char *codegen_constant_name(NativeScaffold *scaffold, const SemanticConstantPlan *constant) {
    SourceSpan name = constant->symbol->declaration->as.declaration.name->name;
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
