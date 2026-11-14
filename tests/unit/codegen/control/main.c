/**
 * @file
 * @brief Verify exact IR site resolution, writer failures and compiled control fragments.
 */
#include <stdint.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../source/common.c"
#include "codegen/control/emission/emission.h"
#include "codegen/control/resolution/site.h"
#include "codegen/control/lists/lists.h"
#include "codegen/internal.h"

/** IR queries are independent of allocation order and unrelated operation kinds. */
static void it_resolves_unique_binding_and_interface_sites_without_a_frontend(void) {
    SemanticStatement statement = {0}, other = {0};
    SemanticBinding binding = {.transfer = TRANSFER_CLONE};
    SemanticSymbol parameter = {.role = SYMBOL_PARAMETER}, result = {.role = SYMBOL_RESULT};
    ControlNode install = {.operation = CONTROL_INSTALL_PARAMETER, .statement = &statement,
        .binding = &binding, .field = &parameter, .selector = 11};
    ControlNode evaluate = {.operation = CONTROL_EVALUATE_INPUT, .statement = &statement,
        .binding = &binding, .selector = 22, .allocated_next = &install};
    ControlNode initialize = {.operation = CONTROL_INITIALIZE_RESULT, .statement = &statement,
        .field = &result, .selector = 33, .allocated_next = &evaluate};
    const ControlNode *found = NULL;
    ControlSiteStatus status = codegen_control_binding_site(&initialize, &statement, &binding, &found);
    assert(status == CONTROL_SITE_FOUND && found == &evaluate);
    status = codegen_control_field_site(&initialize, &statement, &parameter, &found);
    assert(status == CONTROL_SITE_FOUND && found == &install);
    status = codegen_control_field_site(&initialize, &statement, &result, &found);
    assert(status == CONTROL_SITE_FOUND && found == &initialize);
    binding.transfer = TRANSFER_LITERAL;
    status = codegen_control_binding_site(&initialize, &statement, &binding, &found);
    assert(status == CONTROL_SITE_FOUND && found == &install);
    status = codegen_control_binding_site(&initialize, &other, &binding, &found);
    assert(status == CONTROL_SITE_MISSING && found == NULL);
    result.role = SYMBOL_STATE;
    status = codegen_control_field_site(&initialize, &statement, &result, &found);
    assert(status == CONTROL_SITE_MISSING && found == NULL);
}

/** Missing/duplicate IR never chooses the first, nearest or acquisition operation. */
static void it_rejects_missing_and_duplicate_required_sites(void) {
    SemanticStatement statement = {0};
    SemanticBinding binding = {.transfer = TRANSFER_LITERAL};
    SemanticSymbol field = {.role = SYMBOL_PARAMETER};
    ControlNode first = {.operation = CONTROL_INSTALL_PARAMETER, .statement = &statement,
        .binding = &binding, .field = &field, .selector = 1};
    ControlNode second = first;
    second.selector = 2;
    second.allocated_next = &first;
    const ControlNode *found = &first;
    ControlSiteStatus status = codegen_control_binding_site(NULL, &statement, &binding, &found);
    assert(status == CONTROL_SITE_MISSING && found == NULL);
    status = codegen_control_binding_site(&second, &statement, &binding, &found);
    assert(status == CONTROL_SITE_DUPLICATE && found == NULL);
    status = codegen_control_field_site(&second, &statement, &field, &found);
    assert(status == CONTROL_SITE_DUPLICATE && found == NULL);
    first.allocated_next = &second;
    second.allocated_next = NULL;
    status = codegen_control_field_site(&first, &statement, &field, &found);
    assert(status == CONTROL_SITE_DUPLICATE && found == NULL);
}

/** Intrinsic literals select their own staging nodes and never borrow a fake
 * module installation, even if unrelated nodes have the same binding identity.
 */
static void it_resolves_intrinsic_literal_staging_without_module_field_fallback(void) {
    SemanticStatement statement = {.operation_kind = SEMANTIC_LIST_METHOD_OPERATION};
    SemanticBinding binding = {.transfer = TRANSFER_LITERAL};
    ControlNode module = {.operation = CONTROL_INSTALL_PARAMETER, .statement = &statement, .binding = &binding};
    ControlNode stage = {.operation = CONTROL_STAGE_METHOD_INPUT, .statement = &statement,
        .binding = &binding, .allocated_next = &module};
    const ControlNode *found = NULL;
    ControlSiteStatus status = codegen_control_binding_site(&stage, &statement, &binding, &found);
    assert(status == CONTROL_SITE_FOUND && found == &stage);
    status = codegen_control_binding_site(&module, &statement, &binding, &found);
    assert(status == CONTROL_SITE_MISSING && found == NULL);
    ControlNode duplicate = stage;
    duplicate.allocated_next = &stage;
    status = codegen_control_binding_site(&duplicate, &statement, &binding, &found);
    assert(status == CONTROL_SITE_DUPLICATE && found == NULL);
}

/** Every writer propagates a real immediate stream failure to its caller. */
static void it_propagates_unwritable_streams_for_each_operation_family(void) {
    FILE *output = fopen("main.c", "r");
    assert(output != NULL);
    int configured = setvbuf(output, NULL, _IONBF, 0);
    assert(configured == 0);
    assert(!codegen_control_location(output, 1, (SourceSpan){0}, true));
    assert(!codegen_control_jump(output, false, 3));
    assert(!codegen_control_jump(output, true, 0));
    assert(!codegen_control_acquire(output, "demo", 7, true));
    assert(!codegen_control_connect(output, 3, 4));
    assert(!codegen_control_complete(output));
    assert(!codegen_control_release(output));
    assert(!codegen_control_output(output, "demo", true));
    assert(!codegen_control_assignment(output, "demo"));
    assert(!codegen_control_condition(output, "demo", 3, 4));
    assert(!codegen_control_refusal(output));
    assert(!codegen_control_failure(output));
    assert(!codegen_control_save(output, 5));
    assert(!codegen_control_yield(output));
    assert(!codegen_control_resume(output));
    assert(!codegen_control_restart(output));
    IdentityName write = {.internal_name = "W"}, parameters = {.internal_name = "P"};
    IdentityName results = {.internal_name = "R"}, prefix = {.internal_name = "N"};
    ScaffoldType type = {.native_prefix = &prefix};
    type.names[SCAFFOLD_WRITE] = &write;
    type.names[SCAFFOLD_PARAMETERS] = &parameters;
    type.names[SCAFFOLD_RESULTS] = &results;
    SemanticStatement list_statement = {.operation_kind = SEMANTIC_LIST_METHOD_OPERATION};
    StorageFrame list_frame = {.statement = &list_statement};
    assert(!codegen_list_offset(output, &type, &list_frame));
    assert(!codegen_list_frame(output, &type, &list_frame, true, "method"));
    assert(!codegen_emit_native_entry(output, &type));
    int closed = fclose(output);
    assert(closed == 0);
}

/** Preserve physical source positions and omit trace replacement when requested. */
static void it_keeps_original_source_lines_and_preserves_explicit_trace_locations(void) {
    setup_test();
    const unsigned char bytes[] = "header\nbody\n";
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "original.ama", bytes, sizeof(bytes) - 1, &source);
    assert(added == SOURCE_OK);
    FILE *output = tmpfile();
    assert(output != NULL);
    bool emitted = codegen_control_location(output, 17, (SourceSpan){source, 7, 11}, false);
    assert(emitted);
    int flushed = fflush(output);
    assert(flushed == 0);
    rewind(output);
    char text[2048] = {0};
    size_t count = fread(text, 1, sizeof(text) - 1, output);
    assert(count != 0 && strstr(text, "#line 2 \"original.ama\"") != NULL);
    assert(strstr(text, "current_operation") == NULL);
    int closed = fclose(output);
    assert(closed == 0);
    destroy_test();
}

/** Compile/run generated fragments against typed runtime declarations and narrow stubs. */
static void run_probe(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
            "-D_POSIX_C_SOURCE=200809L", "-I" TEST_SDK, "generated/probe.c", "-o", "generated/probe", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    process = fork();
    assert(process >= 0);
    if (process == 0) {
        execl("generated/probe", "generated/probe", (char *)NULL);
        _exit(127);
    }
    waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Check emitted refusal/failure routing, preparation, branch and saved-position effects. */
static void it_executes_emitted_control_fragments_with_real_c_types(void) {
    FILE *output = fopen("generated/probe.c", "w");
    assert(output != NULL);
    int written = fputs(
        "#include <assert.h>\n#include \"runtime/program/calls/call.h\"\n"
        "static AmaProgram ama_program; static AmaHandle ama_current; static AmaStatus ama_status;\n"
        "static AmaPoolCell cell; static AmaPoolCell *ama_cell=&cell; static uint64_t ama_return;\n"
        "static const AmaProgramInvocation demo_Call={0}; static const AmaProgramAccess demo_Access_0={0};\n"
        "static int state,prepared; static bool truth;\n"
        "AmaStatus ama_program_acquire(AmaProgram *p,AmaHandle h,const AmaProgramInvocation *i,bool *r) {\n"
        "(void)p;(void)h;(void)i;*r=state==1;return state==1?ama_status_make(AMA_BUSY,NULL,0):state==2?ama_status_make(AMA_OUT_OF_MEMORY,NULL,0):ama_status_success();}\n"
        "AmaStatus ama_program_prepare(AmaProgram *p,AmaHandle h,const AmaProgramInvocation *i) {\n"
        "(void)p;(void)h;(void)i;prepared++;return state==3?ama_status_make(AMA_OUT_OF_MEMORY,NULL,0):ama_status_success();}\n"
        "AmaStatus ama_program_condition(AmaProgram *p,AmaHandle h,const AmaProgramAccess *a,bool *v) {\n"
        "(void)p;(void)h;(void)a;*v=truth;return ama_status_success();}\n"
        "static int entry(void) {\n", output);
    assert(written != EOF);
    bool emitted = codegen_control_acquire(output, "demo", 7, true);
    assert(emitted);
    written = fputs("return 1; ama_label_7:return 2; ama_project_finish:return 3;}\nstatic int read_only(void) {\n", output);
    assert(written != EOF);
    emitted = codegen_control_acquire(output, "demo", 7, false);
    assert(emitted);
    written = fputs("return 1; ama_label_7:return 2; ama_project_finish:return 3;}\nstatic int branch(void) {\n", output);
    assert(written != EOF);
    emitted = codegen_control_condition(output, "demo", 8, 9);
    assert(emitted);
    written = fputs("ama_label_8:return 8; ama_label_9:return 9; ama_project_finish:return 10;}\nstatic uint64_t resume(void) {\n", output);
    assert(written != EOF);
    emitted = codegen_control_save(output, 44) && codegen_control_resume(output);
    assert(emitted);
    written = fputs("ama_project_resume:return ama_return;}\nstatic void restart(void) {\n", output);
    assert(written != EOF);
    emitted = codegen_control_restart(output);
    assert(emitted);
    written = fputs("}\nint main(void) {state=0;prepared=0;assert(entry()==1&&prepared==1);\n"
        "state=1;prepared=0;assert(entry()==2&&prepared==0);state=2;assert(entry()==3&&prepared==0);\n"
        "state=3;assert(entry()==3&&prepared==1);prepared=0;assert(read_only()==1&&prepared==0);\n"
        "truth=true;assert(branch()==8);truth=false;assert(branch()==9);\n"
        "assert(resume()==44&&!ama_cell->metadata.service.resume.valid);\n"
        "ama_cell->metadata.service.resume=(AmaResumePosition){true,12};restart();assert(!ama_cell->metadata.service.resume.valid);return 0;}\n", output);
    assert(written != EOF);
    int closed = fclose(output);
    assert(closed == 0);
    run_probe();
}

/** Run standalone query/writer tests and execute their generated control behavior. */
void run(void) {
    it_resolves_unique_binding_and_interface_sites_without_a_frontend();
    it_rejects_missing_and_duplicate_required_sites();
    it_resolves_intrinsic_literal_staging_without_module_field_fallback();
    it_propagates_unwritable_streams_for_each_operation_family();
    it_keeps_original_source_lines_and_preserves_explicit_trace_locations();
    it_executes_emitted_control_fragments_with_real_c_types();
}
/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
