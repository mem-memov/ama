#include "codegen/internal.h"

bool codegen_emit_host_finish(FILE *output) {
    return fputs("ama_project_finish:;\n"
        "host->executing=false; host->status=ama_status;\n"
        "if(ama_status.code!=AMA_OK) {\n"
        "    ama_outcome=AMA_PROJECT_FAILED;\n"
        "    if(!ama_program.failure_reported) ama_program_report(&ama_program,ama_status);\n"
        "    (void)ama_project_dispose(host);\n"
        "}\n"
        "return (AmaProjectResult){ama_outcome,host->status};\n"
        "#undef ama_program\n#undef ama_world\n}\n", output) != EOF;
}
