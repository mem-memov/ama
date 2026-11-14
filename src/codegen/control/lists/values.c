/** @file
 * @brief Consume materialized intrinsic values without fake result declarations.
 * [LIST-04](../../../../spec/language/lists.md#list-04) keeps typed frame results
 * alive through their outputs; [LIST-10](../../../../spec/language/lists.md#list-10)
 * grants structural receiver access only to that method's direct binding.
 */
#include "lists.h"

bool codegen_list_output(FILE *output, const ScaffoldType *type,
    const ControlNode *node, const char *name) {
    if (node->binding->position != 0 || fputs("{\n", output) == EOF
        || !codegen_list_frame(output, type, node->frame, true, "list_method")) { return false; }
    return fprintf(output, "if(!list_method->active || !list_method->has_result) { ama_status=ama_status_make(AMA_INVALID_STATE,NULL,0); goto ama_project_finish; }\n"
        "ama_status=ama_program_transfer_authorized(&ama_program,ama_current,&%s_Access_0,&list_method->value,list_method->service);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish; }\n", name) >= 0;
}

bool codegen_list_value_acquire(FILE *output, const ScaffoldType *type,
    const ControlNode *node, const char *name) {
    if (node->statement->intrinsic_result != 0 || fputs("{ bool refused=false;\n", output) == EOF
        || !codegen_list_frame(output, type, node->frame->operation_parent, true, "list_method")) { return false; }
    if (fprintf(output, "if(!list_method->active || !list_method->has_result) { ama_status=ama_status_make(AMA_INVALID_STATE,NULL,0); goto ama_project_finish; }\n"
            "ama_status=ama_program_acquire_value(&ama_program,ama_current,&list_method->value,%s_Call.mode,", name) < 0
        || !codegen_list_offset(output, type, node->frame)
        || fprintf(output, ",&refused); if(ama_status.code!=AMA_OK) { if(refused) goto ama_label_%zu; goto ama_project_finish; }\n",
            node->alternative->selector) < 0) { return false; }
    if (node->statement->syntax->as.call.mode == AST_ENTRY_CALL && fprintf(output,
            "ama_status=ama_program_prepare(&ama_program,ama_current,&%s_Call);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", name) < 0) { return false; }
    return fputs("}\n", output) != EOF;
}
