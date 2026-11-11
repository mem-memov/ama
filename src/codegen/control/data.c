#include "codegen/internal.h"

const char *codegen_control_name(NativeScaffold *scaffold, const ScaffoldType *type, const ControlNode *node) {
    return codegen_indexed_name(scaffold, type->names[SCAFFOLD_WRITE]->internal_name, "Operation", node->selector);
}

static const ControlNode *codegen_binding_node(const ScaffoldType *type, const SemanticStatement *statement,
    const SemanticBinding *binding, const SemanticSymbol *field) {
    for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
        bool install = node->operation == CONTROL_INSTALL_PARAMETER || node->operation == CONTROL_INITIALIZE_RESULT;
        bool evaluate = node->operation == CONTROL_EVALUATE_INPUT;
        if (node->statement == statement && (field != NULL ? install && node->field == field
                : (install || evaluate) && node->binding == binding)) { return node; }
    }
    return NULL;
}

static bool codegen_invocation(NativeScaffold *scaffold, FILE *output, const ScaffoldType *owner, const ControlNode *node, const char *name) {
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
            const ControlNode *site = codegen_binding_node(owner, statement, binding, NULL);
            const char *source = codegen_indexed_name(scaffold, name, "Input", count++);
            const ScaffoldMember *field = codegen_member(callee, binding->field);
            if (field == NULL || source == NULL || fprintf(output, "{.field=%zu,.operation=UINT64_C(%zu),.source=", field->field->slot,
                    site == NULL ? node->selector : site->selector) < 0 || !codegen_source_value(scaffold, output, source, binding->source)
                || fputs("},\n", output) == EOF) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    if (callee->members != NULL) {
        if (fprintf(output, "static const uint64_t %s_Install[] = {", name) < 0) { return false; }
        for (const ScaffoldMember *member = callee->members; member != NULL; member = member->next) {
            const ControlNode *site = codegen_binding_node(owner, statement, NULL, member->field->symbol);
            if (fprintf(output, "UINT64_C(%zu),", site == NULL ? node->selector : site->selector) < 0) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    if (fprintf(output, "static const AmaProgramInvocation %s_Call = {.target=&%s_Access_0,.mode=%s,.input_count=%zu", name, name,
            statement->syntax->as.call.mode == AST_ENTRY_CALL ? "AMA_ACCESS_ENTRY" : statement->syntax->as.call.mode == AST_RESUME_CALL ? "AMA_ACCESS_RESUME" : "AMA_ACCESS_READ", count) < 0) { return false; }
    if (count != 0 && fprintf(output, ",.inputs=%s_Inputs", name) < 0) { return false; }
    if (callee->members != NULL && fprintf(output, ",.installation_operations=%s_Install", name) < 0) { return false; }
    return fputs("};\n", output) != EOF;
}

bool codegen_emit_controls(NativeScaffold *scaffold, FILE *output) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            const char *name = codegen_control_name(scaffold, type, node);
            if (name == NULL) { return false; }
            if (node->operation == CONTROL_ACQUIRE_INSTANCE) {
                if (!codegen_invocation(scaffold, output, type, node, name)) { return false; }
            } else if (node->operation == CONTROL_SELECT_RECEIVER || node->operation == CONTROL_CONDITION || node->operation == CONTROL_TRANSFER_OUTPUT) {
                const SemanticAccess *access = node->operation == CONTROL_TRANSFER_OUTPUT ? node->binding->receiver : node->statement->access;
                if (!codegen_access_definition(scaffold, output, name, access)) { return false; }
                if (node->operation == CONTROL_SELECT_RECEIVER) {
                    const char *source = codegen_indexed_name(scaffold, name, "Source", 0);
                    if (source == NULL || !codegen_source_definition(scaffold, output, source, node->statement->source)
                        || fprintf(output, "static const AmaProgramSource %s_Source = ", name) < 0
                        || !codegen_source_value(scaffold, output, source, node->statement->source) || fputs(";\n", output) == EOF) { return false; }
                }
                if (node->operation == CONTROL_TRANSFER_OUTPUT) {
                    const SemanticSymbol *symbol = node->binding->field;
                    const ScaffoldType *callee = native_scaffold_type(scaffold, symbol->owner);
                    const ScaffoldMember *field = codegen_member(callee, symbol);
                    size_t index = symbol->role == SYMBOL_CONSTANT ? symbol->shared->ordinal : field->field->slot;
                    if (fprintf(output, "static const AmaProgramAccess %s_Output = {.field=%zu,.shared=%s};\n", name, index,
                            symbol->role == SYMBOL_CONSTANT ? "true" : "false") < 0) { return false; }
                }
            }
        }
    }
    return true;
}
