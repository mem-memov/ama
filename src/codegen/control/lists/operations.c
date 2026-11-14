/** @file
 * @brief Emit service admission and supported typed method helpers before outputs.
 * [LIST-10](../../../../spec/language/lists.md#list-10) distinguishes explicit
 * preflight refusal from technical errors; [LIST-09](../../../../spec/language/lists.md#list-09)
 * gives append/insert separate fixed construction/copy signatures.
 */
#include "lists.h"

bool codegen_list_operation(NativeScaffold *scaffold, FILE *output,
    const ScaffoldType *type, const ControlNode *node, const char *name) {
    const SemanticStatement *statement = node->statement;
    if (fputs("{ bool refused=false;\n", output) == EOF) { return false; }
    if (node->operation == CONTROL_ACQUIRE_LIST) {
        const ScaffoldType *owner = native_scaffold_type(scaffold, statement->access->symbol->owner);
        const ScaffoldMember *field = owner == NULL ? NULL : codegen_member(owner, statement->access->symbol);
        if (field == NULL || !codegen_list_frame(output, type, node->frame, false, "list_parent")
            || fprintf(output, "ama_status=ama_program_list_open(&ama_program,ama_current,%s,%zu,list_parent,&refused);\n",
                statement->enclosing_operation == NULL ? "ama_current" : "ama_cell->metadata.service.outgoing",
                field->field->slot) < 0) { return false; }
    } else {
        const char *function = statement->list_method == SEMANTIC_LIST_APPEND ? "ama_program_list_append"
            : statement->list_method == SEMANTIC_LIST_INSERT ? "ama_program_list_insert"
            : statement->list_method == SEMANTIC_LIST_REMOVE ? "ama_program_list_remove"
            : statement->list_method == SEMANTIC_LIST_CLEAR ? "ama_program_list_clear"
            : statement->list_method == SEMANTIC_LIST_SWAP ? "ama_program_list_swap"
            : statement->list_method == SEMANTIC_LIST_LENGTH ? "ama_program_list_length" : NULL;
        if (function == NULL || !codegen_list_frame(output, type, node->frame, true, "list_method")
            || !codegen_list_frame(output, type, node->frame->operation_parent, false, "list_parent")
            || fprintf(output, "ama_status=%s(list_parent,list_method,&%s_List,&refused);\n", function, name) < 0) { return false; }
    }
    return fprintf(output, "if(ama_status.code!=AMA_OK) { if(refused) goto ama_label_%zu; goto ama_project_finish; } }\n",
        node->alternative->selector) >= 0;
}
