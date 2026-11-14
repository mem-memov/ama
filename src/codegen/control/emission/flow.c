/**
 * @file
 * @brief Emit explicit branches, refusal snapshots and terminal transfers.
 *
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) keeps Ama
 * transfers as labels. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives this writer only an output stream and resolved values; it never resolves names.
 * [FLOW-04](../../../../spec/language/control-flow.md#flow-04) converts the
 * terminal result in its protected owner before branching through release paths.
 */
#include "emission.h"

bool codegen_control_jump(FILE *output, bool finish, size_t selector) {
    return finish ? fputs("goto ama_project_finish;\n", output) != EOF
        : fprintf(output, "goto ama_label_%zu;\n", selector) >= 0;
}

/** Both domains preserve the same checked truth/branch protocol. Only resolved
 * header IR selects the result-owner helper; source spelling is never inspected.
 */
static bool condition(FILE *output, const char *name, size_t yes, size_t no, bool selected) {
    return fprintf(output, "{ bool condition=false; ama_status=ama_program_condition%s(&ama_program,ama_current,&%s_Access_0,&condition);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish;\nif(condition) goto ama_label_%zu; else goto ama_label_%zu; }\n", selected ? "_result" : "", name, yes, no) >= 0;
}

bool codegen_control_condition(FILE *output, const char *name, size_t yes, size_t no) {
    return condition(output, name, yes, no, false);
}

bool codegen_control_condition_result(FILE *output, const char *name, size_t yes, size_t no) {
    return condition(output, name, yes, no, true);
}

bool codegen_control_refusal(FILE *output) {
    return fputs("ama_status=ama_status_success();\n", output) != EOF;
}

bool codegen_control_failure(FILE *output) {
    return fputs("goto ama_project_finish;\n", output) != EOF;
}
