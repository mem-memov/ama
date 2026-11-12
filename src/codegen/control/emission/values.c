/**
 * @file
 * @brief Emit ordered result selection and resolved assignments.
 *
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) keeps Ama
 * transfers as labels. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives this writer only an output stream and resolved values; it never resolves names.
 */
#include "emission.h"

bool codegen_control_output(FILE *output, const char *name, bool shared) {
    if (fprintf(output, "{ AmaProgramValue source={0};\n") < 0) { return false; }
    if (shared) {
        if (fprintf(output, "ama_status=ama_program_select_shared(&ama_program,NULL,&%s_Output,&source);\n", name) < 0) { return false; }
    } else if (fprintf(output, "ama_status=ama_program_select(&ama_program,ama_cell->metadata.service.outgoing,&%s_Output,false,&source);\n", name) < 0) { return false; }
    return fprintf(output, "if(ama_status.code==AMA_OK) ama_status=ama_program_transfer(&ama_program,ama_current,&%s_Access_0,&source);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish; }\n", name) >= 0;
}

bool codegen_control_assignment(FILE *output, const char *name) {
    return fprintf(output, "ama_status=ama_program_assign(&ama_program,ama_current,&%s_Access_0,&%s_Source);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", name, name) >= 0;
}
