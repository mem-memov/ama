#include "codegen/internal.h"
#include "source/mapping/directive.h"

static bool codegen_jump(FILE *output, const ControlNode *node) {
    return node != NULL ? fprintf(output, "goto ama_label_%zu;\n", node->selector) >= 0 : fputs("goto ama_project_finish;\n", output) != EOF;
}

static bool codegen_acquire(NativeScaffold *scaffold, FILE *output, const ControlNode *node, const char *name) {
    (void)scaffold;
    if (fprintf(output, "{ bool refused=false; ama_status=ama_program_acquire(&ama_program,ama_current,&%s_Call,&refused);\n"
            "if(ama_status.code!=AMA_OK) { if(refused) goto ama_label_%zu; goto ama_project_finish; }\n", name, node->alternative->selector) < 0) { return false; }
    if (node->statement->syntax->as.call.mode == AST_ENTRY_CALL
        && fprintf(output, "ama_status=ama_program_prepare(&ama_program,ama_current,&%s_Call);\n"
            "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", name) < 0) { return false; }
    return fputs("}\n", output) != EOF;
}

static bool codegen_output(FILE *output, const ControlNode *node, const char *name) {
    if (fprintf(output, "{ AmaProgramValue source={0};\n") < 0) { return false; }
    if (node->statement->syntax->as.call.mode == AST_CONSTANT_READ) {
        if (fprintf(output, "ama_status=ama_program_select_shared(&ama_program,NULL,&%s_Output,&source);\n", name) < 0) { return false; }
    } else if (fprintf(output, "ama_status=ama_program_select(&ama_program,ama_cell->metadata.service.outgoing,&%s_Output,false,&source);\n", name) < 0) { return false; }
    return fprintf(output, "if(ama_status.code==AMA_OK) ama_status=ama_program_transfer(&ama_program,ama_current,&%s_Access_0,&source);\n"
        "if(ama_status.code!=AMA_OK) goto ama_project_finish; }\n", name) >= 0;
}

bool codegen_emit_control(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type, const ControlNode *node) {
    const char *name = codegen_control_name(scaffold, type, node);
    if (name == NULL || fprintf(output, "ama_label_%zu: __attribute__((unused));\n", node->selector) < 0) { return false; }
    if (node->site.source != NULL && source_write_line_directive(output, node->site.source, node->site.start) != SOURCE_OK) { return false; }
    if (fputs("{ AmaStatus located=ama_program_cell(&ama_program,ama_current,&ama_cell);\n"
            "if(located.code!=AMA_OK) { ama_status=located; goto ama_project_finish; } }\n", output) == EOF) { return false; }
    if (node->operation != CONTROL_REFUSAL_SNAPSHOT && node->operation != CONTROL_YIELD && fprintf(output, "ama_cell->metadata.service.current_operation=UINT64_C(%zu);\n", node->selector) < 0) { return false; }
    switch (node->operation) {
        case CONTROL_COMPLETE:
            return fputs("ama_status=ama_program_complete(&ama_program,&ama_current,&ama_return);\n"
                "ama_program.current=ama_current;\nif(ama_status.code!=AMA_OK || ama_return==0) goto ama_project_finish;\ngoto ama_project_return;\n", output) != EOF;
        case CONTROL_ACQUIRE_INSTANCE:
            if (!codegen_acquire(scaffold, output, node, name)) { return false; }
            break;
        case CONTROL_SET_RETURN:
            if (fprintf(output, "ama_status=ama_program_connect(&ama_program,&ama_current,UINT64_C(%zu),UINT64_C(%zu));\n"
                    "ama_program.current=ama_current;\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n", node->resume->selector, node->selector) < 0) { return false; }
            break;
        case CONTROL_NATIVE_ENTRY:
            if (!codegen_emit_native_entry(scaffold, output, type)) { return false; }
            break;
        case CONTROL_REFUSAL_SNAPSHOT:
            if (fputs("ama_status=ama_status_success();\n", output) == EOF) { return false; }
            break;
        case CONTROL_RELEASE_TARGET:
            if (fputs("ama_status=ama_program_release(&ama_program,ama_current);\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n", output) == EOF) { return false; }
            break;
        case CONTROL_TRANSFER_OUTPUT:
            if (!codegen_output(output, node, name)) { return false; }
            break;
        case CONTROL_SELECT_RECEIVER:
            if (fprintf(output, "ama_status=ama_program_assign(&ama_program,ama_current,&%s_Access_0,&%s_Source);\n"
                    "if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", name, name) < 0) { return false; }
            break;
        case CONTROL_CONDITION:
            return fprintf(output, "{ bool condition=false; ama_status=ama_program_condition(&ama_program,ama_current,&%s_Access_0,&condition);\n"
                "if(ama_status.code!=AMA_OK) goto ama_project_finish;\nif(condition) goto ama_label_%zu; else goto ama_label_%zu; }\n",
                name, node->next->selector, node->alternative->selector) >= 0;
        case CONTROL_CAPTURE_ERROR: case CONTROL_CLEANUP_FAILURE: case CONTROL_FAILURE:
            return fputs("goto ama_project_finish;\n", output) != EOF;
        case CONTROL_SAVE_POSITION:
            if (fprintf(output, "ama_cell->metadata.service.resume=(AmaResumePosition){true,UINT64_C(%zu)};\n", node->resume->selector) < 0) { return false; }
            break;
        case CONTROL_YIELD:
            return fputs("ama_status=ama_program_yield(&ama_program,&ama_current,&ama_return);\n"
                "ama_program.current=ama_current;\nif(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
                "if(ama_return==0) goto ama_project_yielded;\ngoto ama_project_return;\n", output) != EOF;
        case CONTROL_RESUME_INSTANCE:
            return fputs("ama_return=ama_cell->metadata.service.resume.selector;\n"
                "ama_cell->metadata.service.resume=(AmaResumePosition){0};\ngoto ama_project_resume;\n", output) != EOF;
        case CONTROL_ENTER_INSTANCE:
            if (fputs("ama_cell->metadata.service.resume=(AmaResumePosition){0};\n", output) == EOF) { return false; }
            break;
        case CONTROL_ACQUIRE_LIST: case CONTROL_LIST_ACTION: case CONTROL_LIST_INSTALL_INPUT:
            return fputs("ama_status=ama_status_make(AMA_INVALID_STATE,\"List actions are not implemented\",32);\n"
                "goto ama_project_finish;\n", output) != EOF;
        case CONTROL_EVALUATE_INPUT: case CONTROL_INSTALL_PARAMETER: case CONTROL_INITIALIZE_RESULT:
        case CONTROL_SELECT_SOURCE: case CONTROL_PREFLIGHT_REPLACEMENT: case CONTROL_PREPARE_REPLACEMENT:
        case CONTROL_COMMIT_REPLACEMENT: case CONTROL_RELEASE_VALUES:
            break;
    }
    return codegen_jump(output, node->next);
}
