#include "codegen/internal.h"

bool codegen_emit_native_entry(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    (void)scaffold;
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (fprintf(output, "{\n%s_Scope scope={0};\n%s parameters={0};\n%s results={0};\n"
            "AmaNativeServices services={.caller_refusal=ama_status_success()};\n"
            "ama_status=%s_BindInterface(&ama_program,ama_current,&scope);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "if(ama_cell->metadata.service.return_link.caller.generation!=0) {\n"
            "AmaPoolCell *caller=NULL; ama_status=ama_program_cell(&ama_program,ama_cell->metadata.service.return_link.caller,&caller);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "services=ama_native_services_snapshot(&caller->metadata.service); }\n", name,
            type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name, name) < 0) { return false; }
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        const SemanticSymbol *symbol = member->field->symbol;
        if (symbol->role == SYMBOL_PARAMETER || symbol->role == SYMBOL_RESULT) {
            if (fprintf(output, "%s.%s=scope.%s.%s;\n", symbol->role == SYMBOL_PARAMETER ? "parameters" : "results",
                    member->identity->internal_name, symbol->role == SYMBOL_PARAMETER ? "read" : "write", member->identity->internal_name) < 0) { return false; }
        }
    }
    return fprintf(output, "ama_world.operation_active=true;\n%s_entry(scope.write.payload,&parameters,&results,&services,&ama_status);\n"
        "ama_world.operation_active=false;\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n}\n", type->native_prefix->internal_name) >= 0;
}
