/** @file
 * @brief Enter the single label dispatcher for an already prepared explicit root.
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) keeps Ama
 * calls as jumps inside this function. [EMBED-03](../../../../spec/implementation/c-library.md#embed-03)
 * permits different exported roots under their context's exclusive executor lease.
 */
#include "codegen/internal.h"
bool codegen_emit_dispatch_begin(NativeScaffold *scaffold, FILE *output) {
    if (fputs(
        "static AmaHostResult ama_project_dispatch(AmaContextView view,uint64_t selector) {\n"
        "AmaPoolCell *ama_cell=NULL; uint64_t ama_return=selector;\n"
        "AmaStatus ama_status=ama_status_success(); AmaHostOutcome ama_outcome=AMA_HOST_COMPLETED;\n"
        "#define ama_world (*view.world)\n#define ama_program (*view.program)\n#define ama_current (view.control->current)\n"
        "ama_status=ama_program_cell(&ama_program,ama_current,&ama_cell);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
        "if(selector!=0) goto ama_project_resume;\n"
        "switch(ama_current.type_id) {\n",output)==EOF) { return false; }
    for (const ScaffoldType *type=scaffold->types; type!=NULL; type=type->next) {
        if (type->plan->entry==NULL) { continue; }
        if (fprintf(output,"case UINT64_C(%zu): goto ama_label_%zu;\n",
            codegen_type_id(scaffold,type),type->plan->entry->selector)<0) { return false; }
    }
    return fputs("default: ama_status=ama_status_make(AMA_TYPE_MISMATCH,NULL,0); goto ama_project_finish;\n}\n",output)!=EOF;
}
