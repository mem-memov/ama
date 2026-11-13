/**
 * @file
 * @brief Emit scoped field/input/output aliases and their complete cleanup.
 *
 * [NAME-05](../../../../spec/implementation/native-interface.md#name-05)
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) supplies
 * the preplanned exact module provider to match its representation-header context.
 * Ordinal access macros name qualified declarations and never leak into the next module context.
 */
#include "native/scaffold/internal.h"

bool scaffold_emit_context(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    if (!native_namespace_begin(output, type->native_prefix)
        || (type->error_provider != 0 && !native_namespace_status(output, type->error_provider))
        || fputs("#define AMA_FIELD(position) AMA_FIELD_##position\n"
            "#define AMA_INPUT(position) AMA_INPUT_##position\n"
            "#define AMA_OUTPUT(position) AMA_OUTPUT_##position\n", output) == EOF) {
        return false;
    }
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        const SemanticSymbol *symbol = member->field->symbol;
        if (fprintf(output, "#define AMA_FIELD_%zu %s\n", symbol->ordinal, member->identity->internal_name) < 0) {
            return false;
        }
        if ((symbol->role == SYMBOL_PARAMETER || symbol->role == SYMBOL_RESULT)
            && fprintf(output, "#define AMA_%s_%zu %s\n", symbol->role == SYMBOL_PARAMETER ? "INPUT" : "OUTPUT",
                symbol->role_ordinal, member->identity->internal_name) < 0) {
            return false;
        }
    }
    return true;
}

bool scaffold_emit_end(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        const SemanticSymbol *symbol = member->field->symbol;
        if (fprintf(output, "#undef AMA_FIELD_%zu\n", symbol->ordinal) < 0) {
            return false;
        }
        if ((symbol->role == SYMBOL_PARAMETER || symbol->role == SYMBOL_RESULT)
            && fprintf(output, "#undef AMA_%s_%zu\n", symbol->role == SYMBOL_PARAMETER ? "INPUT" : "OUTPUT",
                symbol->role_ordinal) < 0) {
            return false;
        }
    }
    return fputs("#undef AMA_FIELD\n#undef AMA_INPUT\n#undef AMA_OUTPUT\n", output) != EOF
        && native_namespace_end(output);
}
