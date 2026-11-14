/**
 * @file
 * @brief Emit ordered field/source descriptors and immutable shared declaration schemas.
 *
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * Semantic roles and ordinals drive preparation order; shared storage remains separate from instance pools.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inferred fields have no creation-time source initializer and use normal default construction.
 */
#include "codegen/internal.h"

/**
 * Emit field initializer data and role/position/operation descriptors in declaration order,
 * followed by shared initializer sources; stream failure stays in staged output.
 */
bool codegen_emit_sources(NativeScaffold *scaffold, FILE *output) {
    const char *const roles[] = {"", "AMA_PROGRAM_PARAMETER", "AMA_PROGRAM_RESULT", "AMA_PROGRAM_STATE"};
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
            const char *base = codegen_indexed_name(scaffold, name, "Field", member->field->slot);
            if (base == NULL || !codegen_initializers(scaffold, output, base, member->field->symbol)) { return false; }
        }
        if (type->members == NULL) { continue; }
        if (fprintf(output, "static const AmaProgramField %s_ProgramFields[] = {\n", name) < 0) { return false; }
        for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
            const SemanticSymbol *symbol = member->field->symbol;
            if (fprintf(output, "{.role=%s, .position=%zu, .operation=UINT64_C(%zu), .initializer_count=%zu", roles[symbol->role], symbol->role_ordinal, codegen_declaration_selector(scaffold, symbol),
                    symbol->initializer_count) < 0) { return false; }
            if (symbol->initializers != NULL && fprintf(output, ", .initializers=%s_Field_%zu_Sources", name, member->field->slot) < 0) { return false; }
            if (fputs("},\n", output) == EOF) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    for (const SharedStorage *shared = scaffold->frontend->plan.shared_first; shared != NULL; shared = shared->next) {
        const char *name = codegen_constant_name(scaffold, shared->constant);
        if (name == NULL || !codegen_initializers(scaffold, output, name, shared->constant->symbol)) { return false; }
    }
    return true;
}

/**
 * Emit immutable shared schemas referring to previously emitted source
 * descriptors. Nonpool types preserve ordinal slots without implicit instances.
 */
bool codegen_emit_program(NativeScaffold *scaffold, FILE *output) {
    if (!codegen_emit_sources(scaffold, output)
        || fputs("static const AmaContextConstant ama_project_constants[] = {\n", output) == EOF) { return false; }
    if (scaffold->frontend->semantic.constant_count == 0 && fputs("{0}\n", output) == EOF) { return false; }
    for (const SharedStorage *shared = scaffold->frontend->plan.shared_first; shared != NULL; shared = shared->next) {
        const char *name = codegen_constant_name(scaffold, shared->constant);
        const SemanticSymbol *symbol = shared->constant->symbol;
        const ScaffoldType *type = native_scaffold_type(scaffold, symbol->type);
        if (name == NULL || fprintf(output, "{.type_id=UINT64_C(%zu), .list=%s, .operation=UINT64_C(%zu), .initializer_count=%zu",
                codegen_type_id(scaffold, type), symbol->list ? "true" : "false",
                codegen_declaration_selector(scaffold, symbol), symbol->initializer_count) < 0) { return false; }
        if (symbol->initializers != NULL && fprintf(output, ", .initializers=%s_Sources", name) < 0) { return false; }
        if (fputs("},\n", output) == EOF) { return false; }
    }
    return fputs("};\n", output) != EOF;
}
