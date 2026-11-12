/**
 * @file
 * @brief Emit acquisition, caller connections, completion and release.
 *
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) keeps Ama
 * transfers as labels. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives this writer only an output stream and resolved values; it never resolves names.
 */
#include "emission.h"

bool codegen_control_acquire(FILE *output, const char *name, size_t alternative, bool entry) {
    if (fprintf(output, "{ bool refused=false; ama_status=ama_program_acquire(&ama_program,ama_current,&%s_Call,&refused);\n"
            "if(ama_status.code!=AMA_OK) { if(refused) goto ama_label_%zu; goto ama_project_finish; }\n", name, alternative) < 0) { return false; }
    if (entry
        && fprintf(output, "ama_status=ama_program_prepare(&ama_program,ama_current,&%s_Call);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", name) < 0) { return false; }
    return fputs("}\n", output) != EOF;
}

bool codegen_control_connect(FILE *output, size_t resume, size_t call_site) {
    return fprintf(output, "ama_status=ama_program_connect(&ama_program,&ama_current,UINT64_C(%zu),UINT64_C(%zu));\n"
        "ama_program.current=ama_current;\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n", resume, call_site) >= 0;
}

bool codegen_control_complete(FILE *output) {
    return fputs("ama_status=ama_program_complete(&ama_program,&ama_current,&ama_return);\n"
        "ama_program.current=ama_current;\nif(ama_status.code!=AMA_OK || ama_return==0) goto ama_project_finish;\ngoto ama_project_return;\n", output) != EOF;
}

bool codegen_control_release(FILE *output) {
    return fputs("ama_status=ama_program_release(&ama_program,ama_current);\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n", output) != EOF;
}
