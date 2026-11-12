/**
 * @file
 * @brief Emit exact nominal field offsets and the retained pool registry.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * C offsetof and real storage types provide layout truth; references retain declared handle locations rather than hidden native edges.
 */
#include "codegen/internal.h"

/**
 * Write field kind/type/offsetof descriptors and the exact executable type registry from
 * validated model members. A false stream result leaves only unpublished partial bytes.
 */
bool codegen_emit_descriptors(NativeScaffold *scaffold, FILE *output) {
    const char *const kinds[] = {"AMA_FIELD_OWNED", "AMA_FIELD_REFERENCE", "AMA_FIELD_OWNING_LIST", "AMA_FIELD_REFERENCE_LIST"};
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        if (type->members != NULL) {
            if (fprintf(output, "static const AmaObjectField %s_Fields[] = {\n", name) < 0) { return false; }
            for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
                const ScaffoldType *child = native_scaffold_type(scaffold, member->field->symbol->type);
                if (fprintf(output, "    {%s, UINT64_C(%zu), offsetof(%s_Storage, %s)},\n",
                        kinds[member->field->form], codegen_type_id(scaffold, child), name, member->identity->internal_name) < 0) { return false; }
            }
            if (fputs("};\n", output) == EOF) { return false; }
        }
    }
    if (fputs("static AmaObjectType *ama_project_registry[] = {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->requires_pool && fprintf(output, "    &%s_Type,\n", type->names[SCAFFOLD_WRITE]->internal_name) < 0) { return false; }
    }
    return fputs("};\n", output) != EOF;
}
