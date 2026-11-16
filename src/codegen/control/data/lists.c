/** @file
 * @brief Describe typed method inputs without inventing module field identities.
 * [LIST-04](../../../../spec/language/lists.md#list-04) fixes intrinsic input
 * ordinals; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * requires each real staging/evaluation site to exist uniquely in validated IR.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03) binds
 * length's sole native constructor to the exact standard integer identity.
 */
#include "data.h"
#include "../resolution/site.h"
#include "semantic/lists/interface.h"

/** Find exactly one declaration-order intrinsic input staging site. Component contract: [LIST-04](../../../../spec/language/lists.md#list-04), [ARCH-05](../../../../spec/implementation/compiler.md#arch-05), [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03). */
static const ControlNode *method_input(const ControlNode *nodes,
    const SemanticStatement *statement, size_t position) {
    const ControlNode *result = NULL;
    for (const ControlNode *node = nodes; node != NULL; node = node->allocated_next) {
        if (node->operation != CONTROL_STAGE_METHOD_INPUT || node->statement != statement || node->position != position) { continue; }
        if (result != NULL) { return NULL; }
        result = node;
    }
    return result;
}

bool codegen_control_list_inputs(NativeScaffold *scaffold, FILE *output,
    const ScaffoldType *owner, const ControlNode *node, const char *name) {
    const SemanticStatement *statement = node->statement;
    size_t count = 0;
    for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
        if (binding->syntax->kind != AST_INPUT || binding->syntax->omitted) { continue; }
        const char *source = codegen_indexed_name(scaffold, name, "Input", count++);
        if (source == NULL || !codegen_source_definition(scaffold, output, source, binding->source)) { return false; }
    }
    if (count != 0) {
        if (fprintf(output, "static const AmaProgramInput %s_Inputs[] = {\n", name) < 0) { return false; }
        size_t index = 0;
        for (const SemanticBinding *binding = statement->bindings; binding != NULL; binding = binding->next) {
            if (binding->syntax->kind != AST_INPUT || binding->syntax->omitted) { continue; }
            const ControlNode *site = NULL;
            const char *source = codegen_indexed_name(scaffold, name, "Input", index++);
            if (source == NULL || codegen_control_binding_site(owner->plan->controls, statement, binding, &site) != CONTROL_SITE_FOUND) {
                return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A list input has no unique resolved evaluation site", NULL);
            }
            if (fprintf(output, "{.field=%zu,.operation=UINT64_C(%zu),.source=", binding->position, site->selector) < 0
                || !codegen_source_value(scaffold, output, source, binding->source) || fputs("},\n", output) == EOF) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    size_t arity = 0;
    while (semantic_list_position(statement->list_method, false, arity) != NULL) { arity++; }
    if (arity != 0) {
        if (fprintf(output, "static const uint64_t %s_Install[] = {", name) < 0) { return false; }
        for (size_t position = 0; position < arity; position++) {
            const ControlNode *site = method_input(owner->plan->controls, statement, position);
            if (site == NULL) {
                return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A list parameter has no unique resolved staging site", NULL);
            }
            if (fprintf(output, "UINT64_C(%zu),", site->selector) < 0) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    const ScaffoldType *integer = native_scaffold_type(scaffold, statement->list_interface.integer);
    if (integer == NULL || fprintf(output, "static const AmaProgramListInputs %s_List = {"
            ".integer_type=UINT64_C(%zu),.operation=UINT64_C(%zu),.count=%zu",
            name, codegen_type_id(scaffold, integer), node->selector, count) < 0) { return false; }
    if (arity != 0 && fprintf(output, ",.installations=%s_Install", name) < 0) { return false; }
    if (statement->list_method == SEMANTIC_LIST_LENGTH) {
        if (!scaffold_size_bridge(integer)) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH,
                "Length requires the fixed standard integer size bridge", NULL);
        }
        if (fprintf(output, ",.from_size=%s_FromSizeValue", integer->names[SCAFFOLD_WRITE]->internal_name) < 0) { return false; }
    }
    if (count != 0 && fprintf(output, ",.inputs=%s_Inputs", name) < 0) { return false; }
    return fputs("};\n", output) != EOF;
}
