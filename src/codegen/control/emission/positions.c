/**
 * @file
 * @brief Emit explicit save, yield, resume and fresh-entry position transitions.
 *
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) keeps Ama
 * transfers as labels. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives this writer only an output stream and resolved values; it never resolves names.
 */
#include "emission.h"

bool codegen_control_save(FILE *output, size_t resume) {
    return fprintf(output, "ama_cell->metadata.service.resume=(AmaResumePosition){true,UINT64_C(%zu)};\n", resume) >= 0;
}

bool codegen_control_yield(FILE *output) {
    return fputs("ama_status=ama_program_yield(&ama_program,&ama_current,&ama_return);\n"
        "ama_program.current=ama_current;\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
        "if(ama_return==0) goto ama_project_yielded;\ngoto ama_project_return;\n", output) != EOF;
}

bool codegen_control_resume(FILE *output) {
    return fputs("ama_return=ama_cell->metadata.service.resume.selector;\n"
        "ama_cell->metadata.service.resume=(AmaResumePosition){0};\ngoto ama_project_resume;\n", output) != EOF;
}

bool codegen_control_restart(FILE *output) {
    return fputs("ama_cell->metadata.service.resume=(AmaResumePosition){0};\n", output) != EOF;
}
