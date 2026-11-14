/** @file
 * @brief Return from completed/yielded/failed label execution without releasing its context lease.
 * [RT-14](../../../../spec/implementation/runtime.md#rt-14) keeps execution
 * outcome separate from host admission. [EMBED-03](../../../../spec/implementation/c-library.md#embed-03)
 * retains exclusive ownership through final validation and result snapshot construction.
 */
#include "codegen/internal.h"
bool codegen_emit_dispatch_finish(FILE *output) {
    return fputs("ama_project_finish:;\n"
        "return (AmaHostResult){.outcome=ama_outcome,.root=view.control->root,.status=ama_status};\n"
        "#undef ama_program\n#undef ama_world\n#undef ama_current\n}\n",output)!=EOF;
}
