/**
 * @file
 * @brief Compose resolved call access, input records and interface-operation tables.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) forbids guessing
 * missing IR. Exact site queries distinguish required operations from metadata
 * that runtime preparation provably never consumes for reads/resume/state fields.
 */
#include "data.h"
#include "../resolution/site.h"

bool codegen_control_invocation(NativeScaffold *scaffold, FILE *output, const ScaffoldType *owner, const ControlNode *node, const char *name) {
    const SemanticStatement *statement = node->statement;
    const ScaffoldType *callee = native_scaffold_type(scaffold, statement->namespace_type);
    if (!codegen_access_definition(scaffold, output, name, statement->access)) { return false; }
    size_t count = 0;
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind != AST_INPUT || binding->syntax->omitted) { continue; }
        const char *source = codegen_indexed_name(scaffold, name, "Input", count++);
        if (source == NULL || !codegen_source_definition(scaffold, output, source, binding->source)) { return false; }
    }
    if (count != 0) {
        if (fprintf(output, "static const AmaProgramInput %s_Inputs[] = {\n", name) < 0) { return false; }
        count = 0;
        for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
            if (binding->syntax->kind != AST_INPUT || binding->syntax->omitted) { continue; }
            const ControlNode *site = NULL;
            if (codegen_control_binding_site(owner->plan->controls, statement, binding, &site) != CONTROL_SITE_FOUND) {
                return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A required input operation is missing or duplicated in resolved IR", NULL);
            }
            const char *source = codegen_indexed_name(scaffold, name, "Input", count++);
            const ScaffoldMember *field = codegen_member(callee, binding->field);
            if (field == NULL || source == NULL || fprintf(output, "{.field=%zu,.operation=UINT64_C(%zu),.source=", field->field->slot,
                    site->selector) < 0 || !codegen_source_value(scaffold, output, source, binding->source)
                || fputs("},\n", output) == EOF) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    if (callee->members != NULL) {
        if (fprintf(output, "static const uint64_t %s_Install[] = {", name) < 0) { return false; }
        for (const ScaffoldMember *member = callee->members; member != NULL; member = member->next) {
            const SemanticSymbol *symbol = member->field->symbol;
            size_t selector = node->selector;
            /* Non-entry tables and persistent fields are never consumed by
             * ama_program_prepare; their explicit inactive marker is the call
             * site. Required interface operations must exist uniquely. */
            if (statement->syntax->as.call.mode == AST_ENTRY_CALL
                && (symbol->role == SYMBOL_PARAMETER || symbol->role == SYMBOL_RESULT)) {
                const ControlNode *site = NULL;
                if (codegen_control_field_site(owner->plan->controls, statement, symbol, &site) != CONTROL_SITE_FOUND) {
                    return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A required interface operation is missing or duplicated in resolved IR", NULL);
                }
                selector = site->selector;
            }
            if (fprintf(output, "UINT64_C(%zu),", selector) < 0) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    if (fprintf(output, "static const AmaProgramInvocation %s_Call = {.target=&%s_Access_0,.mode=%s,.input_count=%zu", name, name,
            statement->syntax->as.call.mode == AST_ENTRY_CALL ? "AMA_ACCESS_ENTRY" : statement->syntax->as.call.mode == AST_RESUME_CALL ? "AMA_ACCESS_RESUME" : "AMA_ACCESS_READ", count) < 0) { return false; }
    if (count != 0 && fprintf(output, ",.inputs=%s_Inputs", name) < 0) { return false; }
    if (callee->members != NULL && fprintf(output, ",.installation_operations=%s_Install", name) < 0) { return false; }
    return fputs("};\n", output) != EOF;
}
