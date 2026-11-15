/**
 * @file
 * @brief Emit the qualified inline native-entry boundary from its resolved type view.
 *
 * [STACK-01](../../../spec/implementation/compiler.md#stack-01) requires verified
 * native-entry inlining. [STACK-03](../../../spec/implementation/compiler.md#stack-03)
 * requires the native call to return normally before Ama continues or yields.
 * [PROTO-09](../../../spec/implementation/native-interface.md#proto-09) checks its
 * separate technical status before any result action.
 * [CABI-03](../../../spec/implementation/native-interface.md#cabi-03) bounds
 * typed views to one checked borrow scope; every post-begin exit closes it.
 * [EMBED-06](../../../spec/implementation/c-library.md#embed-06) copies an
 * independent native-only code provider separately from the caller refusal data.
 * Borrow only resolved names and stream.
 */
#include "codegen/internal.h"

/** Borrow qualified type/member names and emit one operation-scoped native call.
 * Runtime borrows, caller-refusal snapshot and reentry guard stay inside the
 * emitted scope. False reports stream failure; this function executes no native code.
 */
bool codegen_emit_native_entry(FILE *output, const ScaffoldType *type) {
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (fprintf(output, "{\n%s_Scope scope={0};\n%s parameters={0};\n%s results={0};\n"
            "AmaNativeServices services={.caller_refusal=ama_status_success()};\n"
            "AmaNativeBorrowScope borrow={0};\nama_status=ama_native_borrow_begin(&ama_program,ama_current,&borrow);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "ama_status=%s_BindInterface(&borrow,ama_current,&scope);\n"
            "if(ama_status.code==AMA_OK && ama_cell->metadata.service.return_link.caller.generation!=0) {\n"
            "AmaPoolCell *caller=NULL; ama_status=ama_program_cell(&ama_program,ama_cell->metadata.service.return_link.caller,&caller);\n"
            "if(ama_status.code==AMA_OK) services=ama_native_services_snapshot(&caller->metadata.service); }\n"
            "services.code=ama_program.native_code;\n"
            "if(ama_status.code==AMA_OK) {\n", name,
            type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name, name) < 0) { return false; }
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        const SemanticSymbol *symbol = member->field->symbol;
        if (symbol->role == SYMBOL_PARAMETER || symbol->role == SYMBOL_RESULT) {
            if (fprintf(output, "%s.%s=scope.%s.%s;\n", symbol->role == SYMBOL_PARAMETER ? "parameters" : "results",
                    member->identity->internal_name, symbol->role == SYMBOL_PARAMETER ? "read" : "write", member->identity->internal_name) < 0) { return false; }
        }
    }
    return fprintf(output, "%s_entry(scope.write.payload,&parameters,&results,&services,&ama_status);\n"
        "}\nAmaStatus ended=ama_native_borrow_end(&borrow);\n"
        "ama_status=ama_cleanup_primary(ama_status_checked(ama_status),ended);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n}\n", type->native_prefix->internal_name) >= 0;
}
