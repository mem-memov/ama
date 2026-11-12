/**
 * @file
 * @brief Compose assignment and output data from resolved symbols and storage slots.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) uses exact symbol
 * identities and shared ordinals, never C-text inspection or name-based guesses.
 */
#include "data.h"

bool codegen_control_assignment_data(NativeScaffold *scaffold, FILE *output,
    const ControlNode *node, const char *name) {
    const char *source = codegen_indexed_name(scaffold, name, "Source", 0);
    return source != NULL && codegen_source_definition(scaffold, output, source, node->statement->source)
        && fprintf(output, "static const AmaProgramSource %s_Source = ", name) >= 0
        && codegen_source_value(scaffold, output, source, node->statement->source) && fputs(";\n", output) != EOF;
}

bool codegen_control_output_data(NativeScaffold *scaffold, FILE *output,
    const SemanticSymbol *symbol, const char *name) {
    size_t index;
    if (symbol->role == SYMBOL_CONSTANT) {
        index = symbol->shared->ordinal;
    } else {
        const ScaffoldType *callee = native_scaffold_type(scaffold, symbol->owner);
        const ScaffoldMember *field = callee == NULL ? NULL : codegen_member(callee, symbol);
        if (field == NULL) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A resolved output field has no storage member", NULL);
        }
        index = field->field->slot;
    }
    return fprintf(output, "static const AmaProgramAccess %s_Output = {.field=%zu,.shared=%s};\n", name, index,
        symbol->role == SYMBOL_CONSTANT ? "true" : "false") >= 0;
}
